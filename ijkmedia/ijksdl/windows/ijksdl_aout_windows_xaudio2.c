#include <windows.h>
#include <initguid.h>
#include <dsound.h>
#include <xaudio2.h>
#include "ijksdl_aout_windows_xaudio2.h"
#include "../ijksdl_audio.h"
#include "../ijksdl_thread.h"
#include "../ijksdl_aout_internal.h"
#include "../ijksdl_log.h"

//代表创建多少的缓存数据块
//最大为64，为xaudio source queue的最大容量。
#define BUFFER_NUM 64

#define CHECK_XAUDIO2_ERROR(ret__, ...) \
    do { \
    	if (FAILED(ret__)) \
    	{ \
    		ALOGE(__VA_ARGS__); \
    		goto fail; \
    	} \
    } while (0)

#define SAFE_RELEASE(p) if((p)) { (p)->lpVtbl->Release(p); (p)=NULL; }

struct SDL_Aout_Opaque
{
	SDL_cond* wakeup_cond;
	SDL_mutex* wakeup_mutex;
	SDL_Thread* audio_tid;
	SDL_Thread _audio_tid;

	SDL_Thread* notify_tid;
	SDL_Thread _notify_tid;
	SDL_mutex* offset_mutex;

	SDL_AudioSpec    spec;

	volatile bool need_flush;
	
	volatile bool pause_on;
	volatile bool abort_request;

	volatile bool need_set_volume;
	volatile float left_volume;
	volatile float right_volume;

	volatile bool need_set_speed;
	volatile float video_speed;

	IXAudio2* xAudio2;								//xAudio2引擎
	IXAudio2MasteringVoice* xAudio2_master_voice;	//主语音
	IXAudio2SourceVoice* xAudio2_source_voice;		//源语音，每次播放创建。
	WAVEFORMATEX WaveFormat;						//音频格式
	XAUDIO2_BUFFER* XAudio2_Buffer;					//缓存数组，播放队列的基本单位

	int			buffer_size;						//每块数据块缓存大小，与延迟有关。
	uint8_t		* buffer;

	int			offset;								//指向下一个未用缓存pXAudio2Buffer
	uint8_t* queueBuffer[BUFFER_NUM];				//声音数据缓存地址，每个的大小为buffer_size

};

static void set_submitted_xaudio2_buffer(XAUDIO2_BUFFER* XAudio2Buffer, unsigned char* pBuffer, int BufferSize);

//获取音频数据，并打包给xaudio2播放。
static int notify_thread(void* arg)
{
	SDL_Aout*	aout				= (SDL_Aout*)arg;
	SDL_Aout_Opaque* opaque			= aout->opaque;
	SDL_AudioCallback 	audio_cblk	= opaque->spec.callback;
	void*		userdata			= opaque->spec.userdata;
	uint8_t*	buffer				= opaque->buffer;
	int 		buffer_size			= opaque->buffer_size;
	uint8_t**	queueBuffer			= opaque->queueBuffer;
	IXAudio2SourceVoice* xAudio2_source_voice = opaque->xAudio2_source_voice;

	while (!opaque->abort_request)
	{
		audio_cblk(userdata, buffer, buffer_size);
		XAUDIO2_VOICE_STATE state;
		do {
			opaque->xAudio2_source_voice->lpVtbl->GetState(opaque->xAudio2_source_voice, &state, 0);
			//等待xaudio2播放队列空位
		} while (!opaque->abort_request &&state.BuffersQueued >= BUFFER_NUM);

		memcpy(queueBuffer[opaque->offset], buffer, buffer_size);

		HRESULT result = xAudio2_source_voice->lpVtbl->SubmitSourceBuffer(opaque->xAudio2_source_voice, &opaque->XAudio2_Buffer[opaque->offset], NULL);
		CHECK_XAUDIO2_ERROR(result, "%s SubmitSourceBuffer() failed", __func__);
		//下一个，循环队列。
		opaque->offset = (opaque->offset + 1) % BUFFER_NUM;
	}

	xAudio2_source_voice->lpVtbl->Stop(xAudio2_source_voice, 0, XAUDIO2_COMMIT_NOW);
	return 0;
fail:
	return 0;
}

//处理message
static int aout_thread_n(SDL_Aout* aout)
{
	SDL_Aout_Opaque* opaque = aout->opaque;
	int 			buffer_size = opaque->buffer_size;

	LPVOID* audio_buffer = NULL;
	DWORD  	audio_len = 0;
	IXAudio2SourceVoice* xAudio2_source_voice = opaque->xAudio2_source_voice;
	opaque->notify_tid = SDL_CreateThreadEx(&opaque->_notify_tid, notify_thread, aout, "ff_aout_windows_directsound");

	if (!opaque->abort_request && !opaque->pause_on)
	{
		xAudio2_source_voice->lpVtbl->Start(xAudio2_source_voice, 0, XAUDIO2_COMMIT_NOW);
	}

	while (!opaque->abort_request)
	{
		SDL_LockMutex(opaque->wakeup_mutex);
		if (!opaque->abort_request && opaque->pause_on)
		{
			xAudio2_source_voice->lpVtbl->Stop(xAudio2_source_voice, 0, XAUDIO2_COMMIT_NOW);
			while (!opaque->abort_request && opaque->pause_on)
			{
				SDL_CondWaitTimeout(opaque->wakeup_cond, opaque->wakeup_mutex, 1000);
			}

			if (!opaque->abort_request && !opaque->pause_on)
			{
				xAudio2_source_voice->lpVtbl->Start(xAudio2_source_voice, 0, XAUDIO2_COMMIT_NOW);
			}
		}

		if (opaque->need_flush)
		{
			xAudio2_source_voice->lpVtbl->FlushSourceBuffers(xAudio2_source_voice);
			opaque->need_flush = false;
		}

		if (opaque->need_set_volume)
		{
			opaque->need_set_volume = false;
			long volume = (long)(opaque->left_volume + opaque->right_volume) / 2;
			xAudio2_source_voice->lpVtbl->SetVolume(xAudio2_source_voice,volume, XAUDIO2_COMMIT_NOW);
		}

		if (opaque->need_set_speed) {
			//倍速调整
			opaque->xAudio2_source_voice->lpVtbl->SetFrequencyRatio(opaque->xAudio2_source_voice, opaque->video_speed, XAUDIO2_COMMIT_NOW);
			opaque->need_set_speed = false;
		}

		SDL_UnlockMutex(opaque->wakeup_mutex);
	}

	return 0;
}


static int aout_thread(void* arg)
{
	return aout_thread_n(arg);
}


//设置WAVEFORMAT
static void set_wave_format(WAVEFORMATEX *WaveFormat,const SDL_AudioSpec* desired) {
	WaveFormat->wFormatTag = WAVE_FORMAT_PCM;								//PCM格式
	WaveFormat->wBitsPerSample = SDL_AUDIO_BITSIZE(desired->format);		// 字节数
	WaveFormat->nChannels = desired->channels;								//声道数
	WaveFormat->nSamplesPerSec = desired->freq;								//采样率
	WaveFormat->nBlockAlign = (WaveFormat->wBitsPerSample / 8) * desired->channels;//数据块调整
	WaveFormat->nAvgBytesPerSec = WaveFormat->nBlockAlign * desired->freq;	//平均传输速率
	WaveFormat->cbSize = 0;													//附加信息
}

//sourceVoiceBuffer的属性设置。
static void set_submitted_xaudio2_buffer(XAUDIO2_BUFFER* XAudio2Buffer, unsigned char *pBuffer,int BufferSize) {
	XAudio2Buffer->Flags = 0;				//可以设为0或XAUDIO2_END_OF_STREAM，当设为后期时，该XAudio2播放完数据块后自动停止，不再播放下一个数据块
	XAudio2Buffer->AudioBytes = BufferSize;	//音频数据的长度，按字节算
	XAudio2Buffer->pAudioData = pBuffer;	//具体音频数据的地址，unsigned char pBuffer[]
	XAudio2Buffer->PlayBegin = 0;			//播放地址
	XAudio2Buffer->PlayLength = 0;			//播放长度，0为整数据块
	XAudio2Buffer->LoopBegin = 0;			//循环位置
	XAudio2Buffer->LoopLength = 0;			//循环长度，按字节算
	XAudio2Buffer->LoopCount = 0;			//循环次数，0为不循环，255为无限循环
	XAudio2Buffer->pContext = NULL;			//这里的pContext解析该数据块，供回调用，可以是NULL
}


static void aout_close_audio(SDL_Aout* aout)
{
	ALOGI("aout_close_audio()\n");
	SDL_Aout_Opaque* opaque = aout->opaque;
	if (!opaque)
		return;

	SDL_LockMutex(opaque->wakeup_mutex);
	opaque->abort_request = true;
	SDL_CondSignal(opaque->wakeup_cond);
	SDL_UnlockMutex(opaque->wakeup_mutex);


	
	if (opaque->audio_tid)
	{
		SDL_WaitThread(opaque->audio_tid, NULL);
		opaque->audio_tid = NULL;
	}

	if (opaque->notify_tid)
	{
		SDL_WaitThread(opaque->notify_tid, NULL);
		opaque->notify_tid = NULL;
	}

	if (opaque->xAudio2_source_voice) {
		opaque->xAudio2_source_voice->lpVtbl->FlushSourceBuffers(opaque->xAudio2_source_voice);
		opaque->xAudio2_source_voice->lpVtbl->DestroyVoice(opaque->xAudio2_source_voice);
		opaque->xAudio2_source_voice = NULL;
	}

	if (opaque->buffer)
	{
		free(opaque->buffer);
		opaque->buffer = NULL;
		opaque->buffer_size = 0;
	}

	if (opaque->queueBuffer) {
		for (int i = 0; i < BUFFER_NUM; i++) {
			free(opaque->queueBuffer[i]);
			opaque->queueBuffer[i] = NULL;
		}
	}
	

}

//打开音频
static int aout_open_audio(SDL_Aout* aout, const SDL_AudioSpec* desired, SDL_AudioSpec* obtained)
{

	HRESULT result;

	SDL_Aout_Opaque* opaque = aout->opaque;

	opaque->spec = *desired;
	//一开始为了防止延迟，buffer_size不宜设置大。后续随着播放可以增大。当前大小为1/400 秒
	opaque->buffer_size = SDL_AUDIO_BITSIZE(desired->format) * desired->channels * desired->freq / 40 / 400;
	opaque->buffer = (uint8_t*)malloc(opaque->buffer_size);
	
	//每个XAudio2_Buffer绑定一个buffer用于保存音频数据.
	for (int i = 0; i < BUFFER_NUM; i++) {
		opaque->queueBuffer[i] = (uint8_t*)malloc(opaque->buffer_size);
		set_submitted_xaudio2_buffer(&(opaque->XAudio2_Buffer[i]), opaque->queueBuffer[i], opaque->buffer_size);
	}

	opaque->offset = 0;

	set_wave_format(&opaque->WaveFormat, desired);

	result = opaque->xAudio2->lpVtbl->CreateSourceVoice(opaque->xAudio2, &opaque->xAudio2_source_voice, &opaque->WaveFormat, 0, XAUDIO2_DEFAULT_FREQ_RATIO, 0, NULL, NULL);
	CHECK_XAUDIO2_ERROR(result, "%s CreateSourceVoice() failed\n", __func__);

	opaque->xAudio2_source_voice->lpVtbl->FlushSourceBuffers(opaque->xAudio2_source_voice);



	if (obtained) {
		*obtained = *desired;
		obtained->size = opaque->buffer_size;
	}

	opaque->need_set_volume = 0;
	opaque->pause_on = 0;
	opaque->abort_request = false;
	opaque->audio_tid = SDL_CreateThreadEx(&opaque->_audio_tid, aout_thread, aout, "ff_aout_windows_xaudio2");

	return 0;

fail:
	aout_close_audio(aout);
	return -1;

}



static void aout_free_l(SDL_Aout* aout)
{
	if (!aout)
		return;

	SDL_Aout_Opaque* opaque = aout->opaque;
	if (opaque)
	{
		if (!opaque->abort_request)
		{
			aout_close_audio(aout);
		}

		if (opaque->XAudio2_Buffer) {
			free(opaque->XAudio2_Buffer);
			opaque->XAudio2_Buffer = NULL;
		}

		SDL_DestroyCondP(&opaque->wakeup_cond);
		SDL_DestroyMutexP(&opaque->wakeup_mutex);
		SDL_DestroyMutexP(&opaque->offset_mutex);
		
	}


	SDL_Aout_FreeInternal(aout);
}


static void aout_pause_audio(SDL_Aout* aout, int pause_on)
{
	SDL_Aout_Opaque* opaque = aout->opaque;
	SDL_LockMutex(opaque->wakeup_mutex);
	ALOGI("aout_pause_audio(%d)\n", pause_on);
	opaque->pause_on = pause_on;
	if (!pause_on)
		SDL_CondSignal(opaque->wakeup_cond);
	SDL_UnlockMutex(opaque->wakeup_mutex);
}



static void aout_flush_audio(SDL_Aout* aout)
{
	SDL_Aout_Opaque* opaque = aout->opaque;
	SDL_LockMutex(opaque->wakeup_mutex);
	ALOGI("aout_flush_audio()\n");
	opaque->need_flush = true;
	SDL_CondSignal(opaque->wakeup_cond);
	SDL_UnlockMutex(opaque->wakeup_mutex);
}

static void aout_set_volume(SDL_Aout* aout, float left_volume, float right_volume)
{
	SDL_Aout_Opaque* opaque = aout->opaque;
	SDL_LockMutex(opaque->wakeup_mutex);
	ALOGI("aout_set_volume()\n");
	opaque->left_volume = left_volume;
	opaque->right_volume = right_volume;
	opaque->need_set_volume = 1;
	SDL_CondSignal(opaque->wakeup_cond);
	SDL_UnlockMutex(opaque->wakeup_mutex);
}

//音频速度调整
static void aout_set_speed(SDL_Aout* aout, float speed) {
	SDL_Aout_Opaque* opaque = aout->opaque;
	SDL_LockMutex(opaque->wakeup_mutex);
	ALOGI("aout_set_speed()\n");
	opaque->need_set_speed = 1;
	opaque->video_speed = speed;
	SDL_CondSignal(opaque->wakeup_cond);
	SDL_UnlockMutex(opaque->wakeup_mutex);
}

SDL_Aout* SDL_AoutWindows_CreateForXaudio2()
{
	SDL_Aout* aout = SDL_Aout_CreateInternal(sizeof(SDL_Aout_Opaque));
	if (!aout)
		return NULL;
	HRESULT result;

	SDL_Aout_Opaque* opaque = aout->opaque;
	opaque->wakeup_cond = SDL_CreateCond();
	opaque->wakeup_mutex = SDL_CreateMutex();
	opaque->offset_mutex = SDL_CreateMutex();

	//xaudio2 引擎初始化 
	CoInitializeEx(NULL, COINIT_MULTITHREADED);
	result = XAudio2Create(&opaque->xAudio2, 0, XAUDIO2_DEFAULT_PROCESSOR);
	CHECK_XAUDIO2_ERROR(result, "%s XAudio2Create() failed\n", __func__);

	result = opaque->xAudio2->lpVtbl->CreateMasteringVoice(
		opaque->xAudio2 ,
		&opaque->xAudio2_master_voice,
		XAUDIO2_DEFAULT_CHANNELS, 
		XAUDIO2_DEFAULT_SAMPLERATE, 
		0, 
		NULL, 
		NULL,
		AudioCategory_Media
	);//AudioCategory_Media待定。
	CHECK_XAUDIO2_ERROR(result, "%s CreateMasteringVoice() failed\n", __func__);

	aout->free_l = aout_free_l;
	aout->open_audio = aout_open_audio;
	aout->pause_audio = aout_pause_audio;
	aout->flush_audio = aout_flush_audio;
	aout->close_audio = aout_close_audio;
	aout->set_volume = aout_set_volume;
	opaque->XAudio2_Buffer = (XAUDIO2_BUFFER*)malloc(sizeof(XAUDIO2_BUFFER) * BUFFER_NUM);
	return aout;

fail:
	aout_free_l(aout);
	return NULL;
}
