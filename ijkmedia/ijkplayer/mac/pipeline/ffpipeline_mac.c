
#include "ff_ffpipeline.h"
#include "ffpipeline_mac.h"

#include "ffpipenode_ffplay_vdec.h"
#include "ffpipenode_ffplay_adec.h"
#include "../../ff_ffplay.h"
#include "ijksdl_aout_mac_audiounit.h"
#include "ijkplayer_delegate_def.h"

static SDL_Class g_pipeline_class = {
    .name = "ffpipeline_mac_media",
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
    return SDL_AoutMac_CreateForAudioUnit();
}

extern pixvideo_audiotoolbox_delegate g_audiotoolbox_del;
static IJKFF_Pipenode *func_open_audio_decoder(IJKFF_Pipeline *pipeline, FFPlayer *ffp)
{
    VideoState *is = ffp->is;
    AVFormatContext *ic = is->ic;
    
    if(ic->audio_codec_id == AV_CODEC_ID_EAC3 && g_audiotoolbox_del) {
        return g_audiotoolbox_del(ffp);
    } else {
        return ffpipenode_create_audio_decoder_from_ffplay(ffp);
    }
    
}


IJKFF_Pipeline *ffpipeline_create_from_mac(FFPlayer *ffp)
{
    ALOGD("ffpipeline_create_from_mac()\n");
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
    pipeline->func_open_audio_decoder   = func_open_audio_decoder;

    return pipeline;

}



