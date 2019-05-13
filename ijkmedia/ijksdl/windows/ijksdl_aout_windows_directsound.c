#include <windows.h>
#include <initguid.h>
#include <dsound.h>
#include "ijksdl_aout_windows_directsound.h"
#include "../ijksdl_audio.h"
#include "../ijksdl_thread.h"
#include "../ijksdl_aout_internal.h"
#include "../ijksdl_log.h"

//代表创建多少秒的缓存
#define BUFFER_NUM	16

#define CHECK_DIRECTSOUND_ERROR(ret__, ...) \
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
    SDL_cond   *wakeup_cond;
    SDL_mutex  *wakeup_mutex;
    SDL_Thread *audio_tid;
    SDL_Thread _audio_tid;

    SDL_Thread *notify_tid;
    SDL_Thread _notify_tid;
    SDL_mutex  *offset_mutex;

    SDL_AudioSpec    spec;

    IDirectSound8 			*ds_device;
    IDirectSoundBuffer		*ds_buffer;
    IDirectSoundBuffer8 	*ds_buffer8;
    IDirectSoundNotify8		*ds_notify8;
    DSBPOSITIONNOTIFY 		ds_pos_notify[BUFFER_NUM];
    HANDLE 					ds_events[BUFFER_NUM];

    int				buffer_size;
    uint8_t 		*buffer;
    DWORD 			offset;


    volatile bool need_flush;
    volatile bool pause_on;
    volatile bool abort_request;

    volatile bool need_set_volume;
    volatile float left_volume;
    volatile float right_volume;

};



static void directsound_flush(SDL_Aout_Opaque *opaque, IDirectSoundBuffer8 *ds_buffer8, DWORD buffer_size)
{
	if (!ds_buffer8 || !opaque)
		return;


	opaque->need_flush = false;

	DWORD 	audio_len = 0;
	LPVOID 	audio_buffer = NULL;

	SDL_LockMutex(opaque->offset_mutex);
	opaque->offset = opaque->buffer_size;
	SDL_UnlockMutex(opaque->offset_mutex);

	

	IDirectSoundBuffer_Lock(ds_buffer8, 0, buffer_size, &audio_buffer, &audio_len, NULL, NULL, 0);
	memset(audio_buffer, 0, audio_len);
	IDirectSoundBuffer_Unlock(ds_buffer8, audio_buffer, audio_len, NULL, 0);
	IDirectSoundBuffer_SetCurrentPosition(ds_buffer8, 0);
	IDirectSoundBuffer_Play(ds_buffer8, 0, 0, DSBPLAY_LOOPING);
	

	
}


static int notify_thread(void *arg)
{
	SDL_Aout *aout = (SDL_Aout*)arg;
	SDL_Aout_Opaque     	*opaque = aout->opaque;
	IDirectSoundBuffer8 	*ds_buffer8 = opaque->ds_buffer8;
	IDirectSoundNotify8		*ds_notify8 = opaque->ds_notify8;
	SDL_AudioCallback 		audio_cblk = opaque->spec.callback;
	void 					*userdata = opaque->spec.userdata;
	uint8_t 				*buffer = opaque->buffer;
	int 					buffer_size = opaque->buffer_size;

	DWORD 	result = WAIT_OBJECT_0;
	DWORD  	buffer_amount = BUFFER_NUM * opaque->buffer_size;
	LPVOID 	audio_buffer = NULL;
	DWORD  	audio_len = 0;


	opaque->offset = buffer_size;

	
	while (!opaque->abort_request)
	{
		audio_cblk(userdata, buffer, buffer_size);

		SDL_LockMutex(opaque->offset_mutex);
		IDirectSoundBuffer_Lock(ds_buffer8, opaque->offset, buffer_size, &audio_buffer, &audio_len, NULL, NULL, 0);
		memcpy(audio_buffer, buffer, buffer_size);
		opaque->offset += buffer_size;
		opaque->offset %= buffer_amount;
		IDirectSoundBuffer_Unlock(ds_buffer8, audio_buffer, audio_len, NULL, 0);
		SDL_UnlockMutex(opaque->offset_mutex);
		
		result = MsgWaitForMultipleObjects(BUFFER_NUM, opaque->ds_events, FALSE, INFINITE, QS_ALLEVENTS);
	}

	IDirectSoundBuffer_Stop(ds_buffer8);

	return 0;

}


static int aout_thread_n(SDL_Aout *aout)
{
	SDL_Aout_Opaque     	*opaque = aout->opaque;
	IDirectSoundBuffer8 	*ds_buffer8 = opaque->ds_buffer8;
	IDirectSoundNotify8		*ds_notify8 = opaque->ds_notify8;

	int 					buffer_size = opaque->buffer_size;


	for (int i = 0; i < BUFFER_NUM; ++i)
	{
		opaque->ds_events[i] = CreateEvent(NULL, false, false, NULL);
		opaque->ds_pos_notify[i].dwOffset = opaque->buffer_size * i;
		opaque->ds_pos_notify[i].hEventNotify = opaque->ds_events[i];
	}

	IDirectSoundNotify_SetNotificationPositions(ds_notify8, BUFFER_NUM, opaque->ds_pos_notify);



	DWORD  	buffer_amount = BUFFER_NUM * opaque->buffer_size;
	LPVOID 	*audio_buffer = NULL;
	DWORD  	audio_len = 0;

	opaque->notify_tid = SDL_CreateThreadEx(&opaque->_notify_tid, notify_thread, aout, "ff_aout_windows_directsound");


	if (!opaque->abort_request && !opaque->pause_on)
	{
		IDirectSoundBuffer_SetCurrentPosition(ds_buffer8, 0);
		IDirectSoundBuffer_Play(ds_buffer8, 0, 0, DSBPLAY_LOOPING);
	}
	


	while (!opaque->abort_request)
	{
		//IDirectSoundBuffer_GetCurrentPosition(ds_buffer8, &curPlayPos, &curWritePos);
		
		SDL_LockMutex(opaque->wakeup_mutex);
		if (!opaque->abort_request && opaque->pause_on)
		{
			IDirectSoundBuffer_Stop(ds_buffer8);
			while (!opaque->abort_request && opaque->pause_on)
			{
				SDL_CondWaitTimeout(opaque->wakeup_cond, opaque->wakeup_mutex, 1000);
			}

			if (!opaque->abort_request && !opaque->pause_on)
			{
				IDirectSoundBuffer_Play(ds_buffer8, 0, 0, DSBPLAY_LOOPING);
			}
		}

		if (opaque->need_flush)
		{
			directsound_flush(opaque, ds_buffer8, buffer_amount);
		}

		if (opaque->need_set_volume)
		{
			opaque->need_set_volume = false;
			long volume = (long)(opaque->left_volume + opaque->right_volume) / 2;
			IDirectSoundBuffer_SetVolume(ds_buffer8, volume);
		}

		SDL_UnlockMutex(opaque->wakeup_mutex);


		if (opaque->need_flush)
		{
			directsound_flush(opaque, ds_buffer8, buffer_amount);
		}
	}

	return 0;
}


static int aout_thread(void *arg)
{
	return aout_thread_n(arg);
}







static void set_dsbufferdesc(DSBUFFERDESC *desc, const SDL_AudioSpec *desired, int size)
{

	desc->dwSize = sizeof(DSBUFFERDESC);
	desc->dwFlags = DSBCAPS_GLOBALFOCUS | DSBCAPS_CTRLPOSITIONNOTIFY |DSBCAPS_GETCURRENTPOSITION2;
	desc->dwBufferBytes = size * BUFFER_NUM;
	desc->dwReserved = 0;

	desc->lpwfxFormat = (WAVEFORMATEX*)malloc(sizeof(WAVEFORMATEX));
	desc->lpwfxFormat->wFormatTag = WAVE_FORMAT_PCM; 
	desc->lpwfxFormat->nChannels = desired->channels;
	desc->lpwfxFormat->wBitsPerSample = SDL_AUDIO_BITSIZE(desired->format);
	desc->lpwfxFormat->nSamplesPerSec = desired->freq;
	desc->lpwfxFormat->nBlockAlign = (desc->lpwfxFormat->wBitsPerSample/8) * desired->channels;
	desc->lpwfxFormat->nAvgBytesPerSec = desired->freq * desc->lpwfxFormat->nBlockAlign;	
	desc->lpwfxFormat->cbSize = 0;

}


static void aout_close_audio(SDL_Aout *aout)
{
	ALOGI("aout_close_audio()\n");
	SDL_Aout_Opaque *opaque = aout->opaque;
	if (!opaque)
		return;


	SDL_LockMutex(opaque->wakeup_mutex);
	opaque->abort_request = true;
	SDL_CondSignal(opaque->wakeup_cond);
	SDL_UnlockMutex(opaque->wakeup_mutex);

	IDirectSoundBuffer8 	*ds_buffer8 = opaque->ds_buffer8;
	if (ds_buffer8)
	{
		IDirectSoundBuffer_SetCurrentPosition(ds_buffer8, 0);
		IDirectSoundBuffer_Play(ds_buffer8, 0, 0, DSBPLAY_LOOPING);
	}
	

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
}



static int aout_open_audio(SDL_Aout *aout, const SDL_AudioSpec *desired, SDL_AudioSpec *obtained)
{

	HRESULT result;

	SDL_Aout_Opaque  	*opaque = aout->opaque;
	IDirectSound8    	*ds_device = opaque->ds_device;

	opaque->spec = *desired;
	opaque->buffer_size = SDL_AUDIO_BITSIZE(desired->format) * desired->channels * desired->freq / 40;
	opaque->buffer = (uint8_t*)malloc(opaque->buffer_size);

	DSBUFFERDESC buffer_desc;
	memset(&buffer_desc, 0, sizeof(DSBUFFERDESC));
	set_dsbufferdesc(&buffer_desc, desired, opaque->buffer_size);

	result = IDirectSound_CreateSoundBuffer(ds_device, &buffer_desc, &opaque->ds_buffer, NULL);
	CHECK_DIRECTSOUND_ERROR(result, "%s CreateSoundBuffer() failed", __func__);

	result = opaque->ds_buffer->lpVtbl->QueryInterface(opaque->ds_buffer, (const IID *const)&IID_IDirectSoundBuffer8, (LPVOID*)&opaque->ds_buffer8);
	CHECK_DIRECTSOUND_ERROR(result, "%s ds_buffer->QueryInterface() failed", __func__);


	result = opaque->ds_buffer8->lpVtbl->QueryInterface(opaque->ds_buffer8, (const IID *const)&IID_IDirectSoundNotify, (LPVOID*)&opaque->ds_notify8);
	CHECK_DIRECTSOUND_ERROR(result, "%s ds_buffer8->QueryInterface() failed", __func__);


	if (obtained) {
		*obtained = *desired;
		obtained->size = opaque->buffer_size;
	}


	opaque->pause_on = 1;
    opaque->abort_request = false;
    opaque->audio_tid = SDL_CreateThreadEx(&opaque->_audio_tid, aout_thread, aout, "ff_aout_windows_directsound");

	return 0;

fail:
	aout_close_audio(aout);
	return -1; 

}









static void aout_free_l(SDL_Aout *aout)
{
    if (!aout)
        return;

    

    SDL_Aout_Opaque *opaque = aout->opaque;
    if(opaque)
    {
		if (!opaque->abort_request)
		{
			aout_close_audio(aout);
		}

    	if(opaque->buffer)
    	{
    		free(opaque->buffer);
    		opaque->buffer = NULL;
    		opaque->buffer_size = 0;
    	}
    	
    	SDL_DestroyCondP(&opaque->wakeup_cond);
    	SDL_DestroyMutexP(&opaque->wakeup_mutex);
    	SDL_DestroyMutexP(&opaque->offset_mutex);

    	SAFE_RELEASE(opaque->ds_notify8);
    	SAFE_RELEASE(opaque->ds_buffer8);
    	SAFE_RELEASE(opaque->ds_buffer);
    	SAFE_RELEASE(opaque->ds_device);


    	for(int i = 0; i < BUFFER_NUM; ++i)
    		CloseHandle(opaque->ds_events[i]);

    }

    
    SDL_Aout_FreeInternal(aout);
}


static void aout_pause_audio(SDL_Aout *aout, int pause_on)
{
	SDL_Aout_Opaque *opaque = aout->opaque;
	SDL_LockMutex(opaque->wakeup_mutex);
    ALOGI("aout_pause_audio(%d)", pause_on);
    opaque->pause_on = pause_on;
    if (!pause_on)
		SDL_CondSignal(opaque->wakeup_cond);
    SDL_UnlockMutex(opaque->wakeup_mutex);
}



static void aout_flush_audio(SDL_Aout *aout)
{
    SDL_Aout_Opaque *opaque = aout->opaque;
    SDL_LockMutex(opaque->wakeup_mutex);
	ALOGI("aout_flush_audio()");
    opaque->need_flush = true;
    SDL_CondSignal(opaque->wakeup_cond);
    SDL_UnlockMutex(opaque->wakeup_mutex);
}





static void aout_set_volume(SDL_Aout *aout, float left_volume, float right_volume)
{
    SDL_Aout_Opaque *opaque = aout->opaque;
    SDL_LockMutex(opaque->wakeup_mutex);
	ALOGI("aout_flush_audio()");
    opaque->left_volume = left_volume;
    opaque->right_volume = right_volume;
    opaque->need_set_volume = 1;
    SDL_CondSignal(opaque->wakeup_cond);
    SDL_UnlockMutex(opaque->wakeup_mutex);
}



SDL_Aout *SDL_AoutWindows_CreateForDirectSound()
{
	SDL_Aout *aout = SDL_Aout_CreateInternal(sizeof(SDL_Aout_Opaque));
    if (!aout)
        return NULL;
    HRESULT result;

    SDL_Aout_Opaque *opaque = aout->opaque;
    opaque->wakeup_cond = SDL_CreateCond();
    opaque->wakeup_mutex = SDL_CreateMutex();
    opaque->offset_mutex = SDL_CreateMutex();

    result = DirectSoundCreate8(NULL, &opaque->ds_device, NULL);
    CHECK_DIRECTSOUND_ERROR(result, "%s DirectSoundCreate8() failed", __func__);
	result = IDirectSound_SetCooperativeLevel(opaque->ds_device, GetDesktopWindow(), DSSCL_NORMAL);
    CHECK_DIRECTSOUND_ERROR(result, "%s SetCooperativeLevel() failed", __func__);

    aout->free_l = aout_free_l;
    aout->open_audio  = aout_open_audio;
    aout->pause_audio = aout_pause_audio;
    aout->flush_audio = aout_flush_audio;
    aout->close_audio = aout_close_audio;
    aout->set_volume  = aout_set_volume;

    return aout;

fail:
    aout_free_l(aout);
    return NULL;
}