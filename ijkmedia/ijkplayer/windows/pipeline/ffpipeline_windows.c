
#include "../../ff_ffpipeline.h"
#include "ffpipeline_windows.h"
#include "../../pipeline/ffpipenode_ffplay_vdec.h"
#include "../../ff_ffplay.h"
#include "../../../ijkmedia/ijksdl/windows/ijksdl_aout_windows_directsound.h"
#include "../../../ijkmedia/ijksdl/windows/ijksdl_aout_windows_xaudio2.h"
static SDL_Class g_pipeline_class = {
    .name = "ffpipeline_windows_media",
};

typedef struct IJKFF_Pipeline_Opaque {
    FFPlayer      *ffp;

    SDL_Vout      *weak_vout;

    float          left_volume;
    float          right_volume;
} IJKFF_Pipeline_Opaque;


static void func_destroy(IJKFF_Pipeline *pipeline)
{
	// do nothing
}

static IJKFF_Pipenode *func_open_video_decoder(IJKFF_Pipeline *pipeline, FFPlayer *ffp)
{
    return ffpipenode_create_video_decoder_from_ffplay(ffp);
}

static SDL_Aout *func_open_audio_output(IJKFF_Pipeline *pipeline, FFPlayer *ffp)
{
    SDL_Aout* aout;
    if (!ffp->xaudio2) {
        aout = SDL_AoutWindows_CreateForDirectSound();
    }
    else {
        aout = SDL_AoutWindows_CreateForXaudio2();
    }
    return aout;
}



inline static bool check_ffpipeline(IJKFF_Pipeline *pipeline, const char *func_name)
{
    
    if (!pipeline || !pipeline->opaque || !pipeline->opaque_class) {
        ALOGE("%s.%s: invalid pipeline\n", pipeline->opaque_class->name, func_name);
        return false;
    }

    if (pipeline->opaque_class != &g_pipeline_class) {
        ALOGE("%s.%s: unsupported method\n", pipeline->opaque_class->name, func_name);
        return false;
    }

    return true; 
}

IJKFF_Pipeline *ffpipeline_create_from_windows(FFPlayer *ffp)
{
    ALOGD("ffpipeline_create_from_windows()\n");
	IJKFF_Pipeline *pipeline = ffpipeline_alloc(&g_pipeline_class, sizeof(IJKFF_Pipeline_Opaque));
    if (!pipeline)
        return pipeline;

    IJKFF_Pipeline_Opaque *opaque = pipeline->opaque;
    opaque->ffp                   = ffp;
    opaque->left_volume           = 1.0f;
    opaque->right_volume          = 1.0f;


    pipeline->func_destroy              = func_destroy;
    pipeline->func_open_video_decoder   = func_open_video_decoder;
    pipeline->func_open_audio_output    = func_open_audio_output;

    return pipeline;
}



