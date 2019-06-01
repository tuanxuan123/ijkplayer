/*
 * ffpipeline_native_android.c
 *
 * Copyright (c) 2014 Bilibili
 * Copyright (c) 2014 Zhang Rui <bbcallen@gmail.com>
 *
 * This file is part of ijkPlayer.
 *
 * ijkPlayer is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * ijkPlayer is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with ijkPlayer; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#include "ffpipeline_native_android.h"
#include "ffpipenode_native_mediacodec_vdec.h"
#include "../../pipeline/ffpipenode_ffplay_vdec.h"
#include "../../ff_ffplay.h"
#include "ijksdl/android/ijksdl_android.h"

static SDL_Class g_pipeline_class = {
    .name = "ffpipeline_android_native",
};

typedef struct IJKFF_Pipeline_Opaque {
    FFPlayer      *ffp;

    bool         (*mediacodec_select_callback)(void *opaque, ijkmp_mediacodecinfo_context *mcc);
    void          *mediacodec_select_callback_opaque;

    SDL_Vout      *weak_vout;

    float          left_volume;
    float          right_volume;
} IJKFF_Pipeline_Opaque;

static void func_destroy(IJKFF_Pipeline *pipeline)
{
    return;
}

static IJKFF_Pipenode *func_open_video_decoder(IJKFF_Pipeline *pipeline, FFPlayer *ffp)
{
    IJKFF_Pipeline_Opaque *opaque = pipeline->opaque;
    IJKFF_Pipenode        *node = NULL;

    if (ffp->mediacodec_all_videos || ffp->mediacodec_avc || ffp->mediacodec_hevc || ffp->mediacodec_mpeg2)
        node = ffpipenode_create_video_decoder_from_native_mediacodec(ffp, pipeline, opaque->weak_vout);
    if (!node) {
        node = ffpipenode_create_video_decoder_from_ffplay(ffp);
    }

    return node;
}

static SDL_Aout *func_open_audio_output(IJKFF_Pipeline *pipeline, FFPlayer *ffp)
{
    SDL_Aout *aout = NULL;
    if (ffp->opensles) {
        aout = SDL_AoutAndroid_CreateForOpenSLES();
    } else {
        aout = SDL_AoutAndroid_CreateForAudioTrack();
    }
    if (aout)
        SDL_AoutSetStereoVolume(aout, pipeline->opaque->left_volume, pipeline->opaque->right_volume);
    return aout;
}

static IJKFF_Pipenode *func_init_video_decoder(IJKFF_Pipeline *pipeline, FFPlayer *ffp)
{
    IJKFF_Pipeline_Opaque *opaque = pipeline->opaque;
    IJKFF_Pipenode        *node = NULL;

    if (ffp->mediacodec_all_videos || ffp->mediacodec_avc || ffp->mediacodec_hevc || ffp->mediacodec_mpeg2)
        node = ffpipenode_init_decoder_from_native_mediacodec(ffp, pipeline, opaque->weak_vout);

    return node;
}

static int func_config_video_decoder(IJKFF_Pipeline *pipeline, FFPlayer *ffp)
{
    IJKFF_Pipeline_Opaque *opaque = pipeline->opaque;
    int                       ret = 0;

    if (ffp->node_vdec)
        ret = ffpipenode_config_from_native_mediacodec(ffp, pipeline, opaque->weak_vout, ffp->node_vdec);


    return ret;
}


inline static bool check_ffpipeline(IJKFF_Pipeline* pipeline, const char *func_name)
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

IJKFF_Pipeline *ffpipeline_create_from_android(FFPlayer *ffp)
{
    ALOGD("ffpipeline_create_from_android()\n");
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
    pipeline->func_init_video_decoder   = func_init_video_decoder;
    pipeline->func_config_video_decoder = func_config_video_decoder;

    return pipeline;
fail:
    ffpipeline_free_p(&pipeline);
    return NULL;
}





void ffpipeline_set_vout(IJKFF_Pipeline* pipeline, SDL_Vout *vout)
{
    if (!check_ffpipeline(pipeline, __func__))
        return;

    IJKFF_Pipeline_Opaque *opaque = pipeline->opaque;
    opaque->weak_vout = vout;
}



void ffpipeline_set_mediacodec_select_callback(IJKFF_Pipeline* pipeline, bool (*callback)(void *opaque, ijkmp_mediacodecinfo_context *mcc), void *opaque)
{
    ALOGD("%s\n", __func__);
    if (!check_ffpipeline(pipeline, __func__))
        return;

    pipeline->opaque->mediacodec_select_callback        = callback;
    pipeline->opaque->mediacodec_select_callback_opaque = opaque;
}



void ffpipeline_set_volume(IJKFF_Pipeline* pipeline, float left, float right)
{
    ALOGD("%s\n", __func__);
    if (!check_ffpipeline(pipeline, __func__))
        return;

    IJKFF_Pipeline_Opaque *opaque = pipeline->opaque;
    opaque->left_volume  = left;
    opaque->right_volume = right;

    if (opaque->ffp && opaque->ffp->aout) {
        SDL_AoutSetStereoVolume(opaque->ffp->aout, left, right);
    }
}
