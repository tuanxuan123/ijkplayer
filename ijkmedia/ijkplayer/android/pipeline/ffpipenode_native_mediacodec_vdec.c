/*
 * ffpipenode_native_mediacodec_vdec.c
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

#include <media/NdkMediaCodec.h>
#include <media/NdkMediaFormat.h>
#include "ffpipenode_native_mediacodec_vdec.h"
#include "android/ijksdl_android_jni.h"
#include "android/ijkplayer_android_def.h"
#include "android/ijksdl_codec_android_mediadef.h"
#include "ff_ffplay_debug.h"
#include "h264_nal.h"
#include "hevc_nal.h"
#include "mpeg4_esds.h"
#include "../ijkplayer_android_def.h"
#include "../../../ijksdl/ffmpeg/ijksdl_inc_ffmpeg.h"
//#include "../../../../../../Library/Android/sdk/ndk-bundle/sources/android/support/include/stdint.h"


#define AMC_INPUT_TIMEOUT_US  (100 * 1000)
#define AMC_OUTPUT_TIMEOUT_US (100 * 1000)

#define COLOR_FormatYUV420P      19
#define COLOR_FormatYUV420SP     21



typedef struct IJKFF_Pipenode_Opaque {
    FFPlayer                 *ffp;
    IJKFF_Pipeline           *pipeline;
    Decoder                  *decoder;

    ijkmp_mediacodecinfo_context mcc;

    AMediaFormat             *input_format;
    AMediaCodec              *ndk_codec;

    char                      acodec_name[128];
    int                       frame_width;
    int                       frame_height;

    AVCodecContext           *avctx;
    AVCodecParameters        *codecpar;


    SDL_Thread               _enqueue_thread;
    SDL_Thread               *enqueue_thread;

    SDL_mutex                *acodec_mutex;
    SDL_cond                 *acodec_cond;

    volatile bool             acodec_flush_request;
    volatile bool             acodec_reconfigure_request;

    SDL_mutex                *acodec_first_dequeue_output_mutex;
    SDL_cond                 *acodec_first_dequeue_output_cond;
    volatile bool             acodec_first_dequeue_output_request;
    bool                      aformat_need_recreate;

    SDL_mutex                *any_input_mutex;
    SDL_cond                 *any_input_cond;

    int                       input_packet_count;


    SDL_SpeedSampler          sampler;
    size_t                    nal_size;
    bool                      quirk_reconfigure_with_new_codec;

    bool                      is_started;
    volatile bool             abort;
};


static int recreate_format_l(IJKFF_Pipenode *node);
static int reconfigure_codec_l(IJKFF_Pipenode *node);

static void MediaCodec_Stop(IJKFF_Pipenode_Opaque *opaque)
{
    AMediaCodec_stop(opaque->ndk_codec);
    opaque->is_started = false;
}

static bool MediaCodec_Start(IJKFF_Pipenode_Opaque *opaque)
{
    if(AMediaCodec_start(opaque->ndk_codec) != AMEDIA_OK){
        ALOGE("open_video_decoder: MediaCodec_Start failed\n");
        return false;
    }

    opaque->is_started = true;
    return true;
}


static int feed_input_buffer(IJKFF_Pipenode *node, int *enqueue_count)
{
    IJKFF_Pipenode_Opaque *opaque   = node->opaque;
    FFPlayer              *ffp      = opaque->ffp;
    IJKFF_Pipeline        *pipeline = opaque->pipeline;
    VideoState            *is       = ffp->is;
    Decoder               *d        = &is->viddec;
    PacketQueue           *q        = d->queue;

    int                    ret      = 0;
    ssize_t  input_buffer_index = 0;
    int64_t  time_stamp         = 0;


    *enqueue_count = 0;

    if (q->abort_request) {
        return 0;
    }


    H264ConvertState convert_state = {0, 0};

    AVPacket pkt;

    do{
        if (d->queue->nb_packets == 0)
            SDL_CondSignal(d->empty_queue_cond);

        if (ffp_packet_queue_get_or_buffering(ffp, d->queue, &pkt, &d->pkt_serial, &d->finished) < 0) {
            return -1;
        }

        if (ffp_is_flush_packet(&pkt) || opaque->acodec_flush_request) {
            opaque->acodec_flush_request = true;
            SDL_LockMutex(opaque->acodec_mutex);

            if(opaque->is_started){
                if(opaque->input_packet_count > 0){
                    AMediaCodec_flush(opaque->ndk_codec);
                    opaque->input_packet_count = 0;
                }
            }

            opaque->acodec_flush_request = false;
            SDL_CondSignal(opaque->acodec_cond);
            SDL_UnlockMutex(opaque->acodec_mutex);
            d->finished = 0;
            d->next_pts = d->start_pts;
            d->next_pts_tb = d->start_pts_tb;

        }

    }while(ffp_is_flush_packet(&pkt) || d->queue->serial != d->pkt_serial);

    //av_packet_split_side_data(&pkt);
    av_packet_unref(&d->pkt);

    d->pkt_temp = d->pkt = pkt;




    if (opaque->codecpar->codec_id == AV_CODEC_ID_H264 || opaque->codecpar->codec_id == AV_CODEC_ID_HEVC) {
        convert_h264_to_annexb(d->pkt_temp.data, d->pkt_temp.size, opaque->nal_size, &convert_state);
    }


    if(d->pkt_temp.data) {
        //video width or height changed, need do that
        if(opaque->aformat_need_recreate){
            ALOGI("%s: recreate aformat\n", __func__);
            ret = recreate_format_l(node);
            if(ret != 0){
                ALOGE("amc: recreate_format_l failed\n");
                return -1;
            }

            opaque->aformat_need_recreate = false;

            opaque->acodec_reconfigure_request = true;
            SDL_LockMutex(opaque->acodec_mutex);
            ret = reconfigure_codec_l(node);
            opaque->acodec_reconfigure_request = false;
            SDL_CondSignal(opaque->acodec_cond);
            SDL_UnlockMutex(opaque->acodec_mutex);

            if(ret != 0) {
                ALOGE("%s: reconfigure_codec failed\n", __func__);
                return -1;
            }

            SDL_LockMutex(opaque->acodec_first_dequeue_output_mutex);
            while (!q->abort_request &&
                   !opaque->acodec_reconfigure_request &&
                   !opaque->acodec_flush_request &&
                   opaque->acodec_first_dequeue_output_request) {
                SDL_CondWaitTimeout(opaque->acodec_first_dequeue_output_cond, opaque->acodec_first_dequeue_output_mutex, 100);
            }
            SDL_UnlockMutex(opaque->acodec_first_dequeue_output_mutex);

            if (q->abort_request || opaque->acodec_reconfigure_request || opaque->acodec_flush_request) {
                return 0;
            }

        }

        input_buffer_index = AMediaCodec_dequeueInputBuffer(opaque->ndk_codec, -1);

        if(input_buffer_index >= 0) {
            size_t size = 0;
            uint8_t *buf = AMediaCodec_getInputBuffer(opaque->ndk_codec, (size_t)input_buffer_index, &size);

            if(buf && (size > d->pkt_temp.size)){
                memcpy(buf, d->pkt_temp.data, d->pkt_temp.size);

            }
            else{
                ALOGE("AMediaCodec_getInputBuffer size is less than pkt_temp.size\n");
            }
        } else{
            return 0;
        }

        time_stamp = d->pkt_temp.pts;
        if (time_stamp == AV_NOPTS_VALUE && d->pkt_temp.dts != AV_NOPTS_VALUE)
            time_stamp = d->pkt_temp.dts;

        if (time_stamp >= 0) {
            time_stamp = av_rescale_q(time_stamp, is->video_st->time_base, AV_TIME_BASE_Q);
        } else {
            time_stamp = 0;
        }

        if(AMediaCodec_queueInputBuffer(opaque->ndk_codec, input_buffer_index, 0, d->pkt_temp.size, time_stamp, 0) != AMEDIA_OK){
            ALOGE("%s: AMediaCodec_queueInputBuffer failed\n", __func__);
            return -1;
        }


        opaque->input_packet_count++;
        if (enqueue_count)
            ++*enqueue_count;
    } else {
        d->finished = d->pkt_serial;  //video finish
    }


    return 0;
}

static int enqueue_thread_func(void *arg)
{
    IJKFF_Pipenode        *node     = arg;
    IJKFF_Pipenode_Opaque *opaque   = node->opaque;
    FFPlayer              *ffp      = opaque->ffp;
    VideoState            *is       = ffp->is;
    Decoder               *d        = &is->viddec;
    PacketQueue           *q        = d->queue;
    int                    ret      = -1;
    int                    dequeue_count = 0;



    while(!q->abort_request && !opaque->abort){
        ret = feed_input_buffer(node, &dequeue_count);
        if(ret != 0){
            ALOGI("MediaCodec enqueue_thread_func: %s: exit: %d", __func__, ret);
            return ret;
        }
    }

    return 0;
}


static int drain_output_buffer_l(IJKFF_Pipenode *node, int64_t timeUs, int *dequeue_count, AVFrame *frame, int *got_frame)
{
    IJKFF_Pipenode_Opaque *opaque   = node->opaque;
    FFPlayer              *ffp      = opaque->ffp;
    VideoState            *is         = ffp->is;
    int                    ret      = 0;
    AMediaCodecBufferInfo     bufferInfo;
    ssize_t                   output_buffer_index = 0;

    if (dequeue_count)
        *dequeue_count = 0;

    output_buffer_index = AMediaCodec_dequeueOutputBuffer(opaque->ndk_codec, &bufferInfo, timeUs);

    if(output_buffer_index >= 0){
        ffp->stat.vdps = SDL_SpeedSamplerAdd(&opaque->sampler, FFP_SHOW_VDPS_MEDIACODEC, "vdps[MediaCodec]");

        if (dequeue_count)
            ++*dequeue_count;



        size_t size;
        uint8_t *buffer = AMediaCodec_getOutputBuffer(opaque->ndk_codec, output_buffer_index, &size);

        if(buffer){

            AMediaFormat *output_format = AMediaCodec_getOutputFormat(opaque->ndk_codec);

            int color_format = 0;
            int width = 0;
            int height = 0;

            if(!output_format){
                *got_frame = 0;
                AMediaCodec_releaseOutputBuffer(opaque->ndk_codec, output_buffer_index, false);
                ALOGE("%s: AMediaCodec_getOutputFormat failed\n", __func__);
                return -1;
            }

            AMediaFormat_getInt32(output_format, AMEDIAFORMAT_KEY_WIDTH, &width);
            AMediaFormat_getInt32(output_format, AMEDIAFORMAT_KEY_HEIGHT, &height);
            AMediaFormat_getInt32(output_format, AMEDIAFORMAT_KEY_COLOR_FORMAT, &color_format);

            AMediaFormat_delete(output_format);

            frame->width = width;
            frame->height = height;
            frame->sample_aspect_ratio = opaque->codecpar->sample_aspect_ratio;
            frame->pts = av_rescale_q(bufferInfo.presentationTimeUs, AV_TIME_BASE_Q, is->video_st->time_base);

            int frame_size = width * height;

            if(color_format == COLOR_FormatYUV420P) {  //yuv420p
                frame->format = AV_PIX_FMT_YUV420P;
                av_frame_get_buffer(frame, 1);

                memcpy(frame->data[0], buffer, frame_size);
                memcpy(frame->data[1], buffer + frame_size, frame_size/4);
                memcpy(frame->data[2], buffer + frame_size*5/4, frame_size/4);


                *got_frame = 1;
            } else if(color_format == COLOR_FormatYUV420SP) {   //yuv420sp
                frame->format = AV_PIX_FMT_NV12;
                av_frame_get_buffer(frame, 1);

                memcpy(frame->data[0], buffer, frame_size);
                memcpy(frame->data[1], buffer + frame_size, frame_size/2);

                *got_frame = 1;

            }
            else {
                ALOGE("not suport color_format: %d\n", color_format);
            }

            frame->linesize[0] = width;
            frame->linesize[1] = height;
            frame->linesize[2] = width / 2;
            frame->width = opaque->codecpar->width;
            frame->height = opaque->codecpar->height;

        }

        AMediaCodec_releaseOutputBuffer(opaque->ndk_codec, output_buffer_index, false);
    } else {
        *got_frame = 0;
        if(output_buffer_index == AMEDIACODEC__INFO_TRY_AGAIN_LATER){
            ALOGI("output_buffer_index is AMEDIACODEC__INFO_TRY_AGAIN_LATER\n");
        } else if(output_buffer_index == AMEDIACODEC__INFO_OUTPUT_BUFFERS_CHANGED){
            ALOGI("output_buffer_index is AMEDIACODEC__INFO_OUTPUT_BUFFERS_CHANGED\n");
        }else if(output_buffer_index == AMEDIACODEC__INFO_OUTPUT_FORMAT_CHANGED){
            AMediaFormat *output_format = AMediaCodec_getOutputFormat(opaque->ndk_codec);
            if(output_format) {
                int width        = 0;
                int height       = 0;
                int color_format = 0;
                int stride       = 0;
                int slice_height = 0;
                int crop_left    = 0;
                int crop_top     = 0;
                int crop_right   = 0;
                int crop_bottom  = 0;

                AMediaFormat_getInt32(output_format, "width",          &width);
                AMediaFormat_getInt32(output_format, "height",         &height);
                AMediaFormat_getInt32(output_format, "color-format",   &color_format);

                AMediaFormat_getInt32(output_format, "stride",         &stride);
                AMediaFormat_getInt32(output_format, "slice-height",   &slice_height);
                AMediaFormat_getInt32(output_format, "crop-left",      &crop_left);
                AMediaFormat_getInt32(output_format, "crop-top",       &crop_top);
                AMediaFormat_getInt32(output_format, "crop-right",     &crop_right);
                AMediaFormat_getInt32(output_format, "crop-bottom",    &crop_bottom);

                ALOGI("AMEDIACODEC__INFO_OUTPUT_FORMAT_CHANGED\n   width-height: (%d x %d)\n  stride: (%d)\n   crop: (%d, %d, %d, %d)\n",
                      width, height, stride, crop_left, crop_top, crop_right, crop_bottom);
            }
        }else {
            SDL_LockMutex(opaque->any_input_mutex);
            SDL_CondWaitTimeout(opaque->any_input_cond, opaque->any_input_mutex, 1000);
            SDL_UnlockMutex(opaque->any_input_mutex);

        }

    }


    if (opaque->decoder->queue->abort_request)
        return -1;
    else
        return 0;

}

static int drain_output_buffer(IJKFF_Pipenode *node, int64_t timeUs, int *dequeue_count, AVFrame *frame, int *got_frame)
{
    IJKFF_Pipenode_Opaque *opaque = node->opaque;
    SDL_LockMutex(opaque->acodec_mutex);

    if (opaque->acodec_flush_request || opaque->acodec_reconfigure_request) {
        // TODO: invalid picture here?
        // let feed_input_buffer() get mutex
        SDL_CondWaitTimeout(opaque->acodec_cond, opaque->acodec_mutex, 100);
    }

    int ret = drain_output_buffer_l(node, timeUs, dequeue_count, frame, got_frame);

    SDL_UnlockMutex(opaque->acodec_mutex);
    return ret;
}

static void func_destroy(IJKFF_Pipenode *node)
{
    if (!node || !node->opaque)
        return;

    IJKFF_Pipenode_Opaque *opaque = node->opaque;

    SDL_DestroyCondP(&opaque->any_input_cond);
    SDL_DestroyMutexP(&opaque->any_input_mutex);
    SDL_DestroyCondP(&opaque->acodec_cond);
    SDL_DestroyMutexP(&opaque->acodec_mutex);
    SDL_DestroyCondP(&opaque->acodec_first_dequeue_output_cond);
    SDL_DestroyMutexP(&opaque->acodec_first_dequeue_output_mutex);

    if(opaque->input_format){
        AMediaFormat_delete(opaque->input_format);
        opaque->input_format = NULL;
    }


    if(opaque->ndk_codec){
        if(opaque->is_started){
            MediaCodec_Stop(opaque);
        }
        AMediaCodec_delete(opaque->ndk_codec);
        opaque->ndk_codec = NULL;
    }

    avcodec_parameters_free(&opaque->codecpar);

}

static int func_run_sync(IJKFF_Pipenode *node)
{
    IJKFF_Pipenode_Opaque *opaque   = node->opaque;
    FFPlayer              *ffp      = opaque->ffp;
    VideoState            *is       = ffp->is;
    Decoder               *d        = &is->viddec;
    PacketQueue           *q        = d->queue;
    int                    ret      = 0;
    int                    dequeue_count = 0;
    AVFrame               *frame    = NULL;
    int                    got_frame = 0;
    AVRational             tb         = is->video_st->time_base;
    AVRational             frame_rate = av_guess_frame_rate(is->ic, is->video_st, NULL);
    double                 duration;
    double                 pts;


    if(!opaque->ndk_codec)
        return ffp_video_thread(ffp);

    frame = av_frame_alloc();
    if (!frame)
        goto fail;

    opaque->enqueue_thread = SDL_CreateThreadEx(&opaque->_enqueue_thread, enqueue_thread_func, node, "mediacodec_input_thread");
    if (!opaque->enqueue_thread) {
        ALOGE("%s: SDL_CreateThreadEx failed\n", __func__);
        ret = -1;
        goto fail;
    }

    ffp->use_mediacodec = 1;

    while(!q->abort_request){

        int64_t timeUs = opaque->acodec_first_dequeue_output_request ? 0 : AMC_OUTPUT_TIMEOUT_US;
        got_frame = 0;

        ret = drain_output_buffer(node, timeUs, &dequeue_count, frame, &got_frame);
        if (opaque->acodec_first_dequeue_output_request) {
            SDL_LockMutex(opaque->acodec_first_dequeue_output_mutex);
            opaque->acodec_first_dequeue_output_request = false;
            SDL_CondSignal(opaque->acodec_first_dequeue_output_cond);
            SDL_UnlockMutex(opaque->acodec_first_dequeue_output_mutex);
        }

        if (ret != 0) {
            ret = -1;
            goto fail;
        }

        if (got_frame) {
            duration = (frame_rate.num && frame_rate.den ? av_q2d((AVRational){frame_rate.den, frame_rate.num}) : 0);
            pts = (frame->pts == AV_NOPTS_VALUE) ? NAN : frame->pts * av_q2d(tb);

            if (ffp->framedrop > 0 || (ffp->framedrop && ffp_get_master_sync_type(is) != AV_SYNC_VIDEO_MASTER)) {
                ffp->stat.decode_frame_count++;

                if (frame->pts != AV_NOPTS_VALUE) {
                    double dpts = pts;
                    double diff = dpts - ffp_get_master_clock(is);

                    if (!isnan(diff) && fabs(diff) < AV_NOSYNC_THRESHOLD &&
                        diff - is->frame_last_filter_delay < 0 &&
                        is->viddec.pkt_serial == is->vidclk.serial &&
                        is->videoq.nb_packets) {

                        is->frame_drops_early++;
                        is->continuous_frame_drops_early++;

                        if (is->continuous_frame_drops_early > ffp->framedrop) {
                            is->continuous_frame_drops_early = 0;
                        } else {
                            ffp->stat.drop_frame_count++;
                            ffp->stat.drop_frame_rate = (float)(ffp->stat.drop_frame_count) / (float)(ffp->stat.decode_frame_count);

                            av_frame_unref(frame);
                            continue;
                        }
                    }
                }
            }
            frame->flags = USING_MEDIACODEC;
            ffp_queue_picture(ffp, frame, pts, duration, av_frame_get_pkt_pos(frame), is->viddec.pkt_serial);
            av_frame_unref(frame);

        }

    }

fail:
    av_frame_free(&frame);
    opaque->abort = true;
    SDL_WaitThread(opaque->enqueue_thread, NULL);



    if(opaque->ndk_codec){
        SDL_LockMutex(opaque->acodec_mutex);
        SDL_UnlockMutex(opaque->acodec_mutex);

        MediaCodec_Stop(opaque);
        AMediaCodec_delete(opaque->ndk_codec);
        opaque->ndk_codec = NULL;
    }

    return ret;

}

static int func_flush(IJKFF_Pipenode *node)
{
    IJKFF_Pipenode_Opaque *opaque   = node->opaque;

    if (!opaque)
        return -1;

    opaque->acodec_flush_request = true;

    return 0;
}

static int recreate_format_l(IJKFF_Pipenode *node)
{
    IJKFF_Pipenode_Opaque *opaque         = node->opaque;
    FFPlayer              *ffp            = opaque->ffp;

    ALOGI("AMediaFormat: %s, %dx%d\n", opaque->mcc.mime_type, opaque->codecpar->width, opaque->codecpar->height);

    if(opaque->input_format){
        AMediaFormat_delete(opaque->input_format);
        opaque->input_format = NULL;
    }

    opaque->input_format = AMediaFormat_new();
    AMediaFormat_setString(opaque->input_format, AMEDIAFORMAT_KEY_MIME, opaque->mcc.mime_type);
    AMediaFormat_setInt32(opaque->input_format, AMEDIAFORMAT_KEY_WIDTH, opaque->codecpar->width);
    AMediaFormat_setInt32(opaque->input_format, AMEDIAFORMAT_KEY_HEIGHT, opaque->codecpar->height);


    if (opaque->codecpar->extradata && opaque->codecpar->extradata_size > 0) {
        if ((opaque->codecpar->codec_id == AV_CODEC_ID_H264 && opaque->codecpar->extradata[0] == 1)
            ||
            (opaque->codecpar->codec_id == AV_CODEC_ID_HEVC && opaque->codecpar->extradata_size > 3
             && (opaque->codecpar->extradata[0] == 1 || opaque->codecpar->extradata[1] == 1))) {

            size_t   sps_pps_size   = 0;
            size_t   convert_size   = opaque->codecpar->extradata_size + 20;
            uint8_t *convert_buffer = (uint8_t *)calloc(1, convert_size);
            if (!convert_buffer) {
                ALOGE("%s:sps_pps_buffer: alloc failed\n", __func__);
                return -1;
            }

            if (opaque->codecpar->codec_id == AV_CODEC_ID_H264) {
                if (0 != convert_sps_pps(opaque->codecpar->extradata, opaque->codecpar->extradata_size,
                                         convert_buffer, convert_size,
                                         &sps_pps_size, &opaque->nal_size)) {
                    ALOGE("%s:convert_sps_pps: failed\n", __func__);
                    free(convert_buffer);
                    return -1;
                }
            } else {
                if (0 != convert_hevc_nal_units(opaque->codecpar->extradata, opaque->codecpar->extradata_size,
                                                convert_buffer, convert_size,
                                                &sps_pps_size, &opaque->nal_size)) {
                    ALOGE("%s:convert_hevc_nal_units: failed\n", __func__);
                    free(convert_buffer);
                    return -1;
                }
            }

            AMediaFormat_setBuffer(opaque->input_format, "csd-0", convert_buffer, sps_pps_size);
            for(int i = 0; i < sps_pps_size; i+=4) {
                ALOGI("csd-0[%d]: %02x%02x%02x%02x\n", (int)sps_pps_size, (int)convert_buffer[i+0], (int)convert_buffer[i+1], (int)convert_buffer[i+2], (int)convert_buffer[i+3]);
            }

            free(convert_buffer);


        }else if (opaque->codecpar->codec_id == AV_CODEC_ID_MPEG4) {
            size_t esds_dec_dscr_type_length = opaque->codecpar->extradata_size + 0x18;
            size_t esds_es_dscr_type_length = esds_dec_dscr_type_length + 0x08;
            size_t esds_size = esds_es_dscr_type_length + 0x05;
            uint8_t *convert_buffer = (uint8_t *)calloc(1, esds_size);
            restore_mpeg4_esds(opaque->codecpar, opaque->codecpar->extradata, opaque->codecpar->extradata_size, esds_es_dscr_type_length, esds_dec_dscr_type_length, convert_buffer);

            AMediaFormat_setBuffer(opaque->input_format, "csd-0", convert_buffer, esds_size);
            free(convert_buffer);
        }else {
            ALOGE("csd-0: naked\n");
        }
    }else {
        ALOGE("no buffer(%d)\n", opaque->codecpar->extradata_size);
    }

    return 0;

}


static void *create_codec_l(IJKFF_Pipenode *node)
{
    IJKFF_Pipenode_Opaque        *opaque   = node->opaque;
    ijkmp_mediacodecinfo_context *mcc      = &opaque->mcc;

    if(opaque->ndk_codec){
        AMediaCodec_delete(opaque->ndk_codec);
        opaque->ndk_codec = NULL;
    }

    opaque->ndk_codec = AMediaCodec_createDecoderByType(mcc->mime_type);

    if(opaque->ndk_codec){
        strncpy(opaque->acodec_name, mcc->codec_name, sizeof(opaque->acodec_name) / sizeof(*opaque->acodec_name));
        opaque->acodec_name[sizeof(opaque->acodec_name) / sizeof(*opaque->acodec_name) - 1] = 0;

        opaque->quirk_reconfigure_with_new_codec = true;

    }

    opaque->frame_width  = opaque->codecpar->width;
    opaque->frame_height = opaque->codecpar->height;


}

static int reconfigure_codec_l(IJKFF_Pipenode *node)
{
    IJKFF_Pipenode_Opaque *opaque   = node->opaque;

    if(opaque->ndk_codec){
        if(opaque->is_started){
            MediaCodec_Stop(opaque);
        }

        if(opaque->quirk_reconfigure_with_new_codec){
            ALOGI("quirk: reconfigure with new codec");
            create_codec_l(node);
            if (!opaque->ndk_codec) {
                ALOGE("%s:open_video_decoder: create_codec failed\n", __func__);
                return -1;
            }
        }
    }

    if (!opaque->ndk_codec ) {
        create_codec_l(node);
        if (!opaque->ndk_codec) {
            ALOGE("%s:open_video_decoder: create_codec failed\n", __func__);
            return -1;
        }
    }

    if(AMediaCodec_configure(opaque->ndk_codec, opaque->input_format, NULL, NULL, 0) != AMEDIA_OK){
        ALOGE("%s:open_video_decoder: AMediaCodec_configure failed\n", __func__);
        return -1;
    }

    if(!MediaCodec_Start(opaque))
        return -1;

    opaque->acodec_first_dequeue_output_request = true;
    ALOGI("%s:new ndk_codec: %p\n", __func__, opaque->ndk_codec);
    return 0;


}


IJKFF_Pipenode *ffpipenode_init_decoder_from_native_mediacodec(FFPlayer *ffp, IJKFF_Pipeline *pipeline, SDL_Vout *vout)
{
    return NULL;
}


int ffpipenode_config_from_native_mediacodec(FFPlayer *ffp, IJKFF_Pipeline *pipeline, SDL_Vout *vout, IJKFF_Pipenode *node)
{
    return -1;
}

IJKFF_Pipenode *ffpipenode_create_video_decoder_from_native_mediacodec(FFPlayer *ffp, IJKFF_Pipeline *pipeline, SDL_Vout *vout)
{
    if (SDL_Android_GetApiLevel() < IJK_API_21_LOLLIPOP)
        return NULL;

    ALOGD("ffpipenode_create_video_decoder_from_native_mediacodec()\n");

    if (!ffp || !ffp->is)
        return NULL;

    IJKFF_Pipenode *node = ffpipenode_alloc(sizeof(IJKFF_Pipenode_Opaque));
    if (!node)
        return node;

    VideoState            *is     = ffp->is;
    IJKFF_Pipenode_Opaque *opaque = node->opaque;
    opaque->is_started = false;
    int                    ret    = 0;

    node->func_destroy  = func_destroy;
    node->func_run_sync = func_run_sync;
    node->func_flush    = func_flush;

    opaque->pipeline    = pipeline;
    opaque->ffp         = ffp;
    opaque->decoder     = &is->viddec;

    opaque->codecpar = avcodec_parameters_alloc();
    if (!opaque->codecpar)
        goto fail;


    ret = avcodec_parameters_from_context(opaque->codecpar, opaque->decoder->avctx);
    if (ret)
        goto fail;

    switch (opaque->codecpar->codec_id) {
        case AV_CODEC_ID_H264:
            if (!ffp->mediacodec_avc && !ffp->mediacodec_all_videos) {
                ALOGE("%s: MediaCodec: AVC/H264 is disabled. codec_id:%d \n", __func__, opaque->codecpar->codec_id);
                goto fail;
            }
            switch (opaque->codecpar->profile) {
                case FF_PROFILE_H264_BASELINE:
                    ALOGI("%s: MediaCodec: H264_BASELINE: enabled\n", __func__);
                    break;
                case FF_PROFILE_H264_CONSTRAINED_BASELINE:
                    ALOGI("%s: MediaCodec: H264_CONSTRAINED_BASELINE: enabled\n", __func__);
                    break;
                case FF_PROFILE_H264_MAIN:
                    ALOGI("%s: MediaCodec: H264_MAIN: enabled\n", __func__);
                    break;
                case FF_PROFILE_H264_EXTENDED:
                    ALOGI("%s: MediaCodec: H264_EXTENDED: enabled\n", __func__);
                    break;
                case FF_PROFILE_H264_HIGH:
                    ALOGI("%s: MediaCodec: H264_HIGH: enabled\n", __func__);
                    break;
                case FF_PROFILE_H264_HIGH_10:
                    ALOGW("%s: MediaCodec: H264_HIGH_10: disabled\n", __func__);
                    goto fail;
                case FF_PROFILE_H264_HIGH_10_INTRA:
                    ALOGW("%s: MediaCodec: H264_HIGH_10_INTRA: disabled\n", __func__);
                    goto fail;
                case FF_PROFILE_H264_HIGH_422:
                    ALOGW("%s: MediaCodec: H264_HIGH_10_422: disabled\n", __func__);
                    goto fail;
                case FF_PROFILE_H264_HIGH_422_INTRA:
                    ALOGW("%s: MediaCodec: H264_HIGH_10_INTRA: disabled\n", __func__);
                    goto fail;
                case FF_PROFILE_H264_HIGH_444:
                    ALOGW("%s: MediaCodec: H264_HIGH_10_444: disabled\n", __func__);
                    goto fail;
                case FF_PROFILE_H264_HIGH_444_PREDICTIVE:
                    ALOGW("%s: MediaCodec: H264_HIGH_444_PREDICTIVE: disabled\n", __func__);
                    goto fail;
                case FF_PROFILE_H264_HIGH_444_INTRA:
                    ALOGW("%s: MediaCodec: H264_HIGH_444_INTRA: disabled\n", __func__);
                    goto fail;
                case FF_PROFILE_H264_CAVLC_444:
                    ALOGW("%s: MediaCodec: H264_CAVLC_444: disabled\n", __func__);
                    goto fail;
                default:
                    ALOGW("%s: MediaCodec: (%d) unknown profile: disabled\n", __func__, opaque->codecpar->profile);
                    goto fail;
            }
            strcpy(opaque->mcc.mime_type, SDL_AMIME_VIDEO_AVC);
            opaque->mcc.profile = opaque->codecpar->profile;
            opaque->mcc.level   = opaque->codecpar->level;
            break;
        case AV_CODEC_ID_HEVC:
            if (!ffp->mediacodec_hevc && !ffp->mediacodec_all_videos) {
                ALOGE("%s: MediaCodec/HEVC is disabled. codec_id:%d \n", __func__, opaque->codecpar->codec_id);
                goto fail;
            }
            strcpy(opaque->mcc.mime_type, SDL_AMIME_VIDEO_HEVC);
            opaque->mcc.profile = opaque->codecpar->profile;
            opaque->mcc.level   = opaque->codecpar->level;
            break;
        case AV_CODEC_ID_MPEG2VIDEO:
            if (!ffp->mediacodec_mpeg2 && !ffp->mediacodec_all_videos) {
                ALOGE("%s: MediaCodec/MPEG2VIDEO is disabled. codec_id:%d \n", __func__, opaque->codecpar->codec_id);
                goto fail;
            }
            strcpy(opaque->mcc.mime_type, SDL_AMIME_VIDEO_MPEG2VIDEO);
            opaque->mcc.profile = opaque->codecpar->profile;
            opaque->mcc.level   = opaque->codecpar->level;
            break;
        case AV_CODEC_ID_MPEG4:
            if (!ffp->mediacodec_mpeg4 && !ffp->mediacodec_all_videos) {
                ALOGE("%s: MediaCodec/MPEG4 is disabled. codec_id:%d \n", __func__, opaque->codecpar->codec_id);
                goto fail;
            }
            if ((opaque->codecpar->codec_tag & 0x0000FFFF) == 0x00005844) {
                ALOGE("%s: divx is not supported \n", __func__);
                goto fail;
            }
            strcpy(opaque->mcc.mime_type, SDL_AMIME_VIDEO_MPEG4);
            opaque->mcc.profile = opaque->codecpar->profile >= 0 ? opaque->codecpar->profile : 0;
            opaque->mcc.level   = opaque->codecpar->level >= 0 ? opaque->codecpar->level : 1;
            break;

        default:
            ALOGE("%s:create: not H264 or H265/HEVC, codec_id:%d \n", __func__, opaque->codecpar->codec_id);
            goto fail;
    }

    opaque->acodec_mutex                      = SDL_CreateMutex();
    opaque->acodec_cond                       = SDL_CreateCond();
    opaque->acodec_first_dequeue_output_mutex = SDL_CreateMutex();
    opaque->acodec_first_dequeue_output_cond  = SDL_CreateCond();
    opaque->any_input_mutex                   = SDL_CreateMutex();
    opaque->any_input_cond                    = SDL_CreateCond();

    if (!opaque->acodec_mutex || !opaque->acodec_cond || !opaque->acodec_first_dequeue_output_mutex || !opaque->acodec_first_dequeue_output_cond) {
        ALOGE("%s:open_video_decoder: SDL_CreateCond() failed\n", __func__);
        goto fail;
    }

    ret = recreate_format_l(node);
    if (ret != 0) {
        ALOGE("amc: recreate_format_l failed\n");
        goto fail;
    }


    ret = reconfigure_codec_l(node);
    if (ret != 0)
        goto fail;

    ffp_set_video_codec_info(ffp, MEDIACODEC_MODULE_NAME, opaque->mcc.codec_name);



    SDL_SpeedSamplerReset(&opaque->sampler);
    ffp->stat.vdec_type = FFP_PROPV_DECODER_MEDIACODEC;
    return node;

fail:
    ffpipenode_free_p(&node);
    return NULL;


}