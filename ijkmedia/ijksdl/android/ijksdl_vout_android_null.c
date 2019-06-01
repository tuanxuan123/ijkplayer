
#include "ijksdl_vout_android_null.h"
#include "../ijksdl_vout_internal.h"
#include "../ffmpeg/ijksdl_vout_overlay_ffmpeg.h"


struct SDL_Vout_Opaque {
    void *target;
};


static SDL_VoutOverlay *vout_create_overlay_l(int width, int height, int frame_format, SDL_Vout *vout)
{

    return SDL_VoutFFmpeg_CreateOverlay(width, height, frame_format, vout);
}

static SDL_VoutOverlay *vout_create_overlay(int width, int height, int frame_format, SDL_Vout *vout)
{
    SDL_LockMutex(vout->mutex);
    SDL_VoutOverlay *overlay = vout_create_overlay_l(width, height, frame_format, vout);
    SDL_UnlockMutex(vout->mutex);
    return overlay;
}

static void vout_free_l(SDL_Vout *vout)
{
    if (!vout)
        return;

    SDL_Vout_FreeInternal(vout);
}




SDL_Vout *SDL_VoutAndroid_CreateForNull()
{
	SDL_Vout *vout = SDL_Vout_CreateInternal(sizeof(SDL_Vout_Opaque));
	if (!vout)
        return NULL;

	SDL_Vout_Opaque *opaque = vout->opaque;
	opaque->target = NULL;

	vout->create_overlay = vout_create_overlay;
    vout->free_l = vout_free_l;

	return vout;
}

