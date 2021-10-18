#include <windows.h>
#include <initguid.h>
#include <dsound.h>
#include <xaudio2.h>
#include "ijksdl_aout_windows_xaudio2.h"
#include "../ijksdl_audio.h"
#include "../ijksdl_thread.h"
#include "../ijksdl_aout_internal.h"
#include "../ijksdl_log.h"

//���������ٵĻ������ݿ�
//���Ϊ64��Ϊxaudio source queue�����������
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

	IXAudio2* xAudio2;								//xAudio2����
	IXAudio2MasteringVoice* xAudio2_master_voice;	//������
	IXAudio2SourceVoice* xAudio2_source_voice;		//Դ������ÿ�β��Ŵ�����
	WAVEFORMATEX WaveFormat;						//��Ƶ��ʽ
	XAUDIO2_BUFFER* XAudio2_Buffer;					//�������飬���Ŷ��еĻ�����λ

	int			buffer_size;						//ÿ�����ݿ黺���С�����ӳ��йء�
	uint8_t		* buffer;

	int			offset;								//ָ����һ��δ�û���pXAudio2Buffer
	uint8_t* queueBuffer[BUFFER_NUM];				//�������ݻ����ַ��ÿ���Ĵ�СΪbuffer_size

};

static void set_submitted_xaudio2_buffer(XAUDIO2_BUFFER* XAudio2Buffer, unsigned char* pBuffer, int BufferSize);

//��ȡ��Ƶ���ݣ��������xaudio2���š�
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
			//�ȴ�xaudio2���Ŷ��п�λ
		} while (!opaque->abort_request &&state.BuffersQueued >= BUFFER_NUM);

		memcpy(queueBuffer[opaque->offset], buffer, buffer_size);

		HRESULT result = xAudio2_source_voice->lpVtbl->SubmitSourceBuffer(opaque->xAudio2_source_voice, &opaque->XAudio2_Buffer[opaque->offset], NULL);
		CHECK_XAUDIO2_ERROR(result, "%s SubmitSourceBuffer() failed", __func__);
		//��һ����ѭ�����С�
		opaque->offset = (opaque->offset + 1) % BUFFER_NUM;
	}

	xAudio2_source_voice->lpVtbl->Stop(xAudio2_source_voice, 0, XAUDIO2_COMMIT_NOW);
	return 0;
fail:
	return 0;
}

//����message
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
			//���ٵ���
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


//����WAVEFORMAT
static void set_wave_format(WAVEFORMATEX *WaveFormat,const SDL_AudioSpec* desired) {
	WaveFormat->wFormatTag = WAVE_FORMAT_PCM;								//PCM��ʽ
	WaveFormat->wBitsPerSample = SDL_AUDIO_BITSIZE(desired->format);		// �ֽ���
	WaveFormat->nChannels = desired->channels;								//������
	WaveFormat->nSamplesPerSec = desired->freq;								//������
	WaveFormat->nBlockAlign = (WaveFormat->wBitsPerSample / 8) * desired->channels;//���ݿ����
	WaveFormat->nAvgBytesPerSec = WaveFormat->nBlockAlign * desired->freq;	//ƽ����������
	WaveFormat->cbSize = 0;													//������Ϣ
}

//sourceVoiceBuffer���������á�
static void set_submitted_xaudio2_buffer(XAUDIO2_BUFFER* XAudio2Buffer, unsigned char *pBuffer,int BufferSize) {
	XAudio2Buffer->Flags = 0;				//������Ϊ0��XAUDIO2_END_OF_STREAM������Ϊ����ʱ����XAudio2���������ݿ���Զ�ֹͣ�����ٲ�����һ�����ݿ�
	XAudio2Buffer->AudioBytes = BufferSize;	//��Ƶ���ݵĳ��ȣ����ֽ���
	XAudio2Buffer->pAudioData = pBuffer;	//������Ƶ���ݵĵ�ַ��unsigned char pBuffer[]
	XAudio2Buffer->PlayBegin = 0;			//���ŵ�ַ
	XAudio2Buffer->PlayLength = 0;			//���ų��ȣ�0Ϊ�����ݿ�
	XAudio2Buffer->LoopBegin = 0;			//ѭ��λ��
	XAudio2Buffer->LoopLength = 0;			//ѭ�����ȣ����ֽ���
	XAudio2Buffer->LoopCount = 0;			//ѭ��������0Ϊ��ѭ����255Ϊ����ѭ��
	XAudio2Buffer->pContext = NULL;			//�����pContext���������ݿ飬���ص��ã�������NULL
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

//����Ƶ
static int aout_open_audio(SDL_Aout* aout, const SDL_AudioSpec* desired, SDL_AudioSpec* obtained)
{

	HRESULT result;

	SDL_Aout_Opaque* opaque = aout->opaque;

	opaque->spec = *desired;
	//һ��ʼΪ�˷�ֹ�ӳ٣�buffer_size�������ô󡣺������Ų��ſ������󡣵�ǰ��СΪ1/400 ��
	opaque->buffer_size = SDL_AUDIO_BITSIZE(desired->format) * desired->channels * desired->freq / 40 / 400;
	opaque->buffer = (uint8_t*)malloc(opaque->buffer_size);
	
	//ÿ��XAudio2_Buffer��һ��buffer���ڱ�����Ƶ����.
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

//��Ƶ�ٶȵ���
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

	//xaudio2 �����ʼ�� 
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
	);//AudioCategory_Media������
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
