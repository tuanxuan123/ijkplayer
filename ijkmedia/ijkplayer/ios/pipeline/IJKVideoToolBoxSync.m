/*****************************************************************************
 * IJKVideoToolBox.m
 *****************************************************************************
 *
 * copyright (c) 2014 Zhou Quan <zhouqicy@gmail.com>
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

#include "IJKVideoToolBoxSync.h"
#include "ijksdl_vout_overlay_videotoolbox.h"
#include "ffpipeline_ios.h"
#include <mach/mach_time.h>
#include "libavformat/avc.h"
#include "libavformat/hevc.h"
#include "h264_sps_parser.h"
#include "ijkplayer/ff_ffplay_debug.h"
#import <CoreMedia/CoreMedia.h>
#import <CoreFoundation/CoreFoundation.h>
#import <CoreVideo/CVHostTime.h>
#import <Foundation/Foundation.h>
#include <stdatomic.h>
#import <VideoToolbox/VideoToolbox.h>
#include "ff_ffinc.h"
#include "ff_fferror.h"
#include "ff_ffmsg.h"
#include "ijksdl/ios/ijksdl_vout_overlay_videotoolbox.h"


#define IJK_VTB_FCC_AVCC   SDL_FOURCC('C', 'c', 'v', 'a')

#define MAX_REF_FRAMES   6
#define MAX_PKT_QUEUE_DEEP   280
#define H264_PS_COUNT    2
#define H265_PS_COUNT    3


typedef struct sample_info {
    double  sort;
    int64_t  dts;
    int64_t  pts;
    int     serial;
    int     sar_num;
    int     sar_den;
} sample_info;

typedef struct avframe_queue {
    AVFrame pic;
    int serial;
    int64_t sort;
    volatile struct avframe_queue *nextframe;
} avframe_queue;

typedef struct VTBFormatDesc
{
    CMFormatDescriptionRef      fmt_desc;
    int32_t                     max_ref_frames;
    bool                        convert_bytestream;
    bool                        convert_3byteTo4byteNALSize;
} VTBFormatDesc;

struct Ijk_VideoToolBox_Opaque {
    FFPlayer                   *ffp;
    VTDecompressionSessionRef   vt_session;

    AVCodecParameters          *codecpar;
    VTBFormatDesc               fmt_desc;
    
    SDL_Thread                  *queue_frame_thread;
    SDL_Thread                  _queue_frame_thread;

    volatile bool               refresh_request;
    volatile bool               new_seg_flag;
    volatile bool               refresh_session;

    pthread_mutex_t             m_queue_mutex;
    volatile avframe_queue      *m_sort_queue;
    volatile int32_t            m_queue_depth;
    sample_info                 sample_info;

    SDL_SpeedSampler            sampler;
    int                         m_buffer_deep;
    AVPacket                    m_buffer_packet[MAX_PKT_QUEUE_DEEP];
    
    int                         serial;
    bool                        dealloced;
    bool                        is_eof;
};



static void vtbformat_destroy(VTBFormatDesc *fmt_desc);
static int  vtbformat_init(VTBFormatDesc *fmt_desc, AVCodecParameters *codecpar);


static const char *vtb_get_error_string(OSStatus status) {
    switch (status) {
        case kVTInvalidSessionErr:                      return "kVTInvalidSessionErr";
        case kVTVideoDecoderBadDataErr:                 return "kVTVideoDecoderBadDataErr";
        case kVTVideoDecoderUnsupportedDataFormatErr:   return "kVTVideoDecoderUnsupportedDataFormatErr";
        case kVTVideoDecoderMalfunctionErr:             return "kVTVideoDecoderMalfunctionErr";
        default:                                        return "UNKNOWN";
    }
}

static void SortQueuePop(Ijk_VideoToolBox_Opaque* context)
{
    if (!context->m_sort_queue || context->m_queue_depth <= 0) {
        return;
    }

    pthread_mutex_lock(&context->m_queue_mutex);
    if (!context->m_sort_queue || context->m_queue_depth <= 0) {
        pthread_mutex_unlock(&context->m_queue_mutex);
        return;
    }
    volatile avframe_queue *top_frame = context->m_sort_queue;
    context->m_sort_queue = context->m_sort_queue->nextframe;
    context->m_queue_depth--;
    pthread_mutex_unlock(&context->m_queue_mutex);
    
    if(top_frame) {
        CVBufferRelease(top_frame->pic.opaque);
        free((void*)top_frame);
    }
    
}

static void SortQueuePush(Ijk_VideoToolBox_Opaque *ctx, avframe_queue *newFrame)
{
    pthread_mutex_lock(&ctx->m_queue_mutex);
    volatile avframe_queue *queueWalker = ctx->m_sort_queue;
    if (!queueWalker || (newFrame->sort < queueWalker->sort)) {
        newFrame->nextframe = queueWalker;
        ctx->m_sort_queue = newFrame;
    } else {
        bool frameInserted = false;
        volatile avframe_queue *nextFrame = NULL;
        while (!frameInserted) {
            nextFrame = queueWalker->nextframe;
            if (!nextFrame || (newFrame->sort < nextFrame->sort)) {
                newFrame->nextframe = nextFrame;
                queueWalker->nextframe = newFrame;
                frameInserted = true;
            }
            queueWalker = nextFrame;
        }
    }
    ctx->m_queue_depth++;

    pthread_mutex_unlock(&ctx->m_queue_mutex);
}

static void ResetPktBuffer(Ijk_VideoToolBox_Opaque* context) {
    for (int i = 0 ; i < context->m_buffer_deep; i++) {
        av_packet_unref(&context->m_buffer_packet[i]);
    }
    context->m_buffer_deep = 0;
    memset(context->m_buffer_packet, 0, sizeof(context->m_buffer_packet));
}

static void DuplicatePkt(Ijk_VideoToolBox_Opaque* context, const AVPacket* pkt) {
    if (context->m_buffer_deep >= MAX_PKT_QUEUE_DEEP) {
        ResetPktBuffer(context);
    }
    AVPacket* avpkt = &context->m_buffer_packet[context->m_buffer_deep];

    av_copy_packet(avpkt, pkt);

    context->m_buffer_deep++;

}

static void CFDictionarySetSInt32(CFMutableDictionaryRef dictionary, CFStringRef key, SInt32 numberSInt32)
{
    CFNumberRef number;
    number = CFNumberCreate(NULL, kCFNumberSInt32Type, &numberSInt32);
    CFDictionarySetValue(dictionary, key, number);
    CFRelease(number);
}


static CMSampleBufferRef CreateSampleBufferFrom(CMFormatDescriptionRef fmt_desc, void *demux_buff, size_t demux_size)
{
    OSStatus status;
    CMBlockBufferRef newBBufOut = NULL;
    CMSampleBufferRef sBufOut = NULL;

    status = CMBlockBufferCreateWithMemoryBlock(NULL,
                                                demux_buff,
                                                demux_size,
                                                kCFAllocatorNull,
                                                NULL,
                                                0,
                                                demux_size,
                                                0,
                                                &newBBufOut);

    if (!status) {
        status = CMSampleBufferCreate(NULL,
                                      newBBufOut,
                                      TRUE,
                                      0,
                                      0,
                                      fmt_desc,
                                      1,
                                      0,
                                      NULL,
                                      0,
                                      NULL,
                                      &sBufOut);
    }

    if (newBBufOut)
        CFRelease(newBBufOut);
    if (status == 0) {
        return sBufOut;
    } else {
        return NULL;
    }
}

static bool GetVTBPicture(Ijk_VideoToolBox_Opaque* context, AVFrame* pVTBPicture)
{
    if (!context->m_sort_queue || context->m_queue_depth < 1) {
        return false;
    }
    
    if(!context->is_eof && context->m_queue_depth < MAX_REF_FRAMES)
        return false;
    
    
    pthread_mutex_lock(&context->m_queue_mutex);

    volatile avframe_queue *top_frame = context->m_sort_queue;
    *pVTBPicture        = top_frame->pic;
    pVTBPicture->opaque = top_frame->pic.opaque;
    
    context->m_sort_queue = context->m_sort_queue->nextframe;
    context->m_queue_depth--;
    pthread_mutex_unlock(&context->m_queue_mutex);
    
    free((void*)top_frame);

    return true;
}

static void QueuePicture(Ijk_VideoToolBox_Opaque* ctx) {
    
    AVFrame picture = {0};
    if (ctx && GetVTBPicture(ctx, &picture)) {
        if(!ctx->ffp || !ctx->ffp->is)
            return;
        
        AVRational tb = ctx->ffp->is->video_st->time_base;
        AVRational frame_rate = av_guess_frame_rate(ctx->ffp->is->ic, ctx->ffp->is->video_st, NULL);
        double duration = (frame_rate.num && frame_rate.den ? av_q2d((AVRational) {frame_rate.den, frame_rate.num}) : 0);
        double pts = (picture.pts == AV_NOPTS_VALUE) ? NAN : picture.pts * av_q2d(tb);

        if (ctx->ffp->mediacodec_surface) {
            picture.format = IJK_AV_PIX_FMT__TEXTURE_TOOLBOX;
        } else {
            picture.format = IJK_AV_PIX_FMT__VIDEO_TOOLBOX;
        }
        
        ffp_queue_picture(ctx->ffp, &picture, pts, duration, 0, ctx->ffp->is->viddec.pkt_serial);

        CVBufferRelease(picture.opaque);
        
    } else {
        usleep(10000);
    }
}


static void VTDecoderCallback(void *decompressionOutputRefCon,
                       void *sourceFrameRefCon,
                       OSStatus status,
                       VTDecodeInfoFlags infoFlags,
                       CVImageBufferRef imageBuffer,
                       CMTime presentationTimeStamp,
                       CMTime presentationDuration)
{
    @autoreleasepool {
        Ijk_VideoToolBox_Opaque *ctx = (Ijk_VideoToolBox_Opaque*)decompressionOutputRefCon;
        if (!ctx)
            return;

        FFPlayer   *ffp         = ctx->ffp;
        VideoState *is          = ffp->is;
        avframe_queue *newFrame    = NULL;

        sample_info *sample_info = &ctx->sample_info;

        newFrame = (avframe_queue *)mallocz(sizeof(avframe_queue));
        if (!newFrame) {
            NSLog(@"VTB: create new frame fail: out of memory");
            goto failed;
        }

        newFrame->pic.pts        = sample_info->pts;
        newFrame->pic.pkt_dts    = sample_info->dts;
        newFrame->pic.sample_aspect_ratio.num = sample_info->sar_num;
        newFrame->pic.sample_aspect_ratio.den = sample_info->sar_den;
        newFrame->serial     = sample_info->serial;
        newFrame->nextframe  = NULL;
        
        if (newFrame->pic.pts != AV_NOPTS_VALUE) {
            newFrame->sort    = newFrame->pic.pts;
        } else {
            newFrame->sort    = newFrame->pic.pkt_dts;
            newFrame->pic.pts = newFrame->pic.pkt_dts;
        }

        if (ctx->dealloced || is->abort_request || is->viddec.queue->abort_request)
            goto failed;

        if (status != 0) {
            NSLog(@"decode callback %d %s\n", (int)status, vtb_get_error_string(status));
            goto failed;
        }

        if (ctx->refresh_session) {
            goto failed;
        }

        if (newFrame->serial != ctx->serial) {
            goto failed;
        }

        if (imageBuffer == NULL) {
            ALOGI("imageBuffer null\n");
            goto failed;
        }

        ffp->stat.vdps = SDL_SpeedSamplerAdd(&ctx->sampler, FFP_SHOW_VDPS_VIDEOTOOLBOX, "vdps[VideoToolbox]");


        OSType format_type = CVPixelBufferGetPixelFormatType(imageBuffer);
        if (format_type != kCVPixelFormatType_420YpCbCr8BiPlanarVideoRange) {
            NSLog(@"format_type error \n");
            goto failed;
        }
        if (kVTDecodeInfo_FrameDropped & infoFlags) {
            NSLog(@"droped\n");
            goto failed;
        }

        // FIXME: duplicated code
        {
            double dpts = NAN;

            if (newFrame->pic.pts != AV_NOPTS_VALUE)
                dpts = av_q2d(is->video_st->time_base) * newFrame->pic.pts;

            if (ffp->framedrop>0 || (ffp->framedrop && ffp_get_master_sync_type(is) != AV_SYNC_VIDEO_MASTER)) {
                ffp->stat.decode_frame_count++;
                if (newFrame->pic.pts != AV_NOPTS_VALUE) {
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
                            // drop too late frame
                            goto failed;
                        }
                    }
                }
            }
        }

        if (CVPixelBufferIsPlanar(imageBuffer)) {
            newFrame->pic.width  = (int)CVPixelBufferGetWidthOfPlane(imageBuffer, 0);
            newFrame->pic.height = (int)CVPixelBufferGetHeightOfPlane(imageBuffer, 0);
        } else {
            newFrame->pic.width  = (int)CVPixelBufferGetWidth(imageBuffer);
            newFrame->pic.height = (int)CVPixelBufferGetHeight(imageBuffer);
        }


        newFrame->pic.opaque = CVBufferRetain(imageBuffer);
        SortQueuePush(ctx, newFrame);

        return;
    failed:
        if (newFrame) {
            free(newFrame);
        }
        return;
    }

}

static void vtbsession_destroy(Ijk_VideoToolBox_Opaque *context)
{
    if (!context)
        return;

    vtbformat_destroy(&context->fmt_desc);

    if (context->vt_session) {
        VTDecompressionSessionWaitForAsynchronousFrames(context->vt_session);
        VTDecompressionSessionInvalidate(context->vt_session);
        CFRelease(context->vt_session);
        context->vt_session = NULL;
    }
}

static VTDecompressionSessionRef vtbsession_create(Ijk_VideoToolBox_Opaque* context)
{
    FFPlayer *ffp = context->ffp;
    int       ret = 0;
    int       width  = context->codecpar->width;
    int       height = context->codecpar->height;

    VTDecompressionSessionRef vt_session = NULL;
    CFMutableDictionaryRef destinationPixelBufferAttributes;
    VTDecompressionOutputCallbackRecord outputCallback;
    OSStatus status;

    vtbformat_destroy(&context->fmt_desc);
    ret = vtbformat_init(&context->fmt_desc, context->codecpar);

    if (ffp->vtb_max_frame_width > 0 && width > ffp->vtb_max_frame_width) {
        double w_scaler = (float)ffp->vtb_max_frame_width / width;
        width = ffp->vtb_max_frame_width;
        height = height * w_scaler;
    }

    ALOGI("after scale width %d height %d \n", width, height);

    destinationPixelBufferAttributes = CFDictionaryCreateMutable(kCFAllocatorDefault,
                                                                 3,
                                                                 &kCFTypeDictionaryKeyCallBacks,
                                                                 &kCFTypeDictionaryValueCallBacks);
    CFDictionarySetSInt32(destinationPixelBufferAttributes,
                          kCVPixelBufferPixelFormatTypeKey, kCVPixelFormatType_420YpCbCr8BiPlanarVideoRange);
    CFDictionarySetSInt32(destinationPixelBufferAttributes,
                          kCVPixelBufferWidthKey, width);
    CFDictionarySetSInt32(destinationPixelBufferAttributes,
                          kCVPixelBufferHeightKey, height);
    
    outputCallback.decompressionOutputCallback = VTDecoderCallback;
    outputCallback.decompressionOutputRefCon = context;
    status = VTDecompressionSessionCreate(
                                          kCFAllocatorDefault,
                                          context->fmt_desc.fmt_desc,
                                          NULL,
                                          destinationPixelBufferAttributes,
                                          &outputCallback,
                                          &vt_session);

    if (status != noErr) {
        NSError* error = [NSError errorWithDomain:NSOSStatusErrorDomain code:status userInfo:nil];
        NSLog(@"VTDecompressionSessionCreate Error %@", [error description]);
        ALOGI("%s - failed with status = (%d)\n", __FUNCTION__, (int)status);
    }
    CFRelease(destinationPixelBufferAttributes);

    memset(&context->sample_info, 0, sizeof(struct sample_info));
    
    return vt_session;
}



static int decode_video_internal(Ijk_VideoToolBox_Opaque* context, AVCodecContext *avctx, const AVPacket *avpkt, int* got_picture_ptr)
{
    FFPlayer *ffp                   = context->ffp;
    OSStatus status                 = 0;
    uint32_t decoder_flags          = 0;
    sample_info *sample_info        = NULL;
    CMSampleBufferRef sample_buff   = NULL;
    AVIOContext *pb                 = NULL;
    int demux_size                  = 0;
    uint8_t *demux_buff             = NULL;
    uint8_t *pData                  = avpkt->data;
    int iSize                       = avpkt->size;
    int64_t pts                     = avpkt->pts;
    int64_t dts                     = avpkt->dts;

    if (!context) {
        goto failed;
    }

    if (context->refresh_session) {
        decoder_flags |= kVTDecodeFrame_DoNotOutputFrame;
        // ALOGI("flag :%d flag %d \n", decoderFlags,avpkt->flags);
    }

    if (context->refresh_request) {
        while (context->m_queue_depth > 0) {
            SortQueuePop(context);
        }
        vtbsession_destroy(context);
        memset(&context->sample_info, 0, sizeof(struct sample_info));
        
        context->vt_session = vtbsession_create(context);
        if (!context->vt_session)
            goto failed;
        context->refresh_request = false;
    }

    if (pts == AV_NOPTS_VALUE) {
        pts = dts;
    }

    if (context->fmt_desc.convert_bytestream) {
        // ALOGI("the buffer should m_convert_byte\n");

        if(avio_open_dyn_buf(&pb) < 0) {
            goto failed;
        }
        ff_avc_parse_nal_units(pb, pData, iSize);

        demux_size = avio_close_dyn_buf(pb, &demux_buff);

        // ALOGI("demux_size:%d\n", demux_size);
        if (demux_size == 0) {
            goto failed;
        }
        sample_buff = CreateSampleBufferFrom(context->fmt_desc.fmt_desc, demux_buff, demux_size);
    } else if (context->fmt_desc.convert_3byteTo4byteNALSize) {
        if (avio_open_dyn_buf(&pb) < 0) {
            goto failed;
        }

        uint32_t nal_size;
        uint8_t *end = avpkt->data + avpkt->size;
        uint8_t *nal_start = pData;
        while (nal_start < end) {
            nal_size = AV_RB24(nal_start);
            avio_wb32(pb, nal_size);
            nal_start += 3;
            avio_write(pb, nal_start, nal_size);
            nal_start += nal_size;
        }

        demux_size = avio_close_dyn_buf(pb, &demux_buff);

        sample_buff = CreateSampleBufferFrom(context->fmt_desc.fmt_desc, demux_buff, demux_size);
    } else {
        sample_buff = CreateSampleBufferFrom(context->fmt_desc.fmt_desc, pData, iSize);
    }
    if (!sample_buff) {
        if (demux_size) {
            av_free(demux_buff);
        }
        NSLog(@"%s - CreateSampleBufferFrom failed", __FUNCTION__);
        goto failed;
    }

    if (avpkt->flags & AV_PKT_FLAG_NEW_SEG) {
        context->new_seg_flag = true;
    }

    sample_info = &context->sample_info;
    if (!sample_info) {
        NSLog(@"%s, failed to peek frame_info\n", __FUNCTION__);
        goto failed;
    }
    

    sample_info->pts    = pts;
    sample_info->dts    = dts;
    sample_info->serial = context->serial;
    sample_info->sar_num = avctx->sample_aspect_ratio.num;
    sample_info->sar_den = avctx->sample_aspect_ratio.den;

    status = VTDecompressionSessionDecodeFrame(context->vt_session, sample_buff, decoder_flags, (void*)sample_info, 0);
    
    if (status != noErr) {
        NSLog(@"decodeFrame failed! stauts is:%d  dec:%s\n", (int)status, vtb_get_error_string(status));
        if (status == kVTInvalidSessionErr || status == kVTVideoDecoderMalfunctionErr) {
            context->refresh_session = true;
            
            vtbsession_destroy(context);
            memset(&context->sample_info, 0, sizeof(struct sample_info));
            
            context->vt_session = vtbsession_create(context);
            if (!context->vt_session)
                return -1;
        }
        
        goto failed;
    }
        
    
    if (ffp->is->videoq.abort_request)
        goto failed;


    if (sample_buff) {
        CFRelease(sample_buff);
    }
    if (demux_size) {
        av_free(demux_buff);
    }

    *got_picture_ptr = 1;
    return 0;
    
failed:
    if (sample_buff) {
        CFRelease(sample_buff);
    }
    if (demux_size) {
        av_free(demux_buff);
    }
    *got_picture_ptr = 0;
    return -1;
}




static int decode_video(Ijk_VideoToolBox_Opaque* context, AVCodecContext *avctx, AVPacket *avpkt, int* got_picture_ptr)
{
    int      ret            = 0;
    AVCodecParameters *codecpar = context->codecpar;
    int codec_id = codecpar->codec_id;
    uint8_t *extradata      = codecpar->extradata;
    int      extradata_size = codecpar->extradata_size;
    
    FFPlayer    *ffp = context->ffp;
    VideoState  *is = ffp->is;
    Decoder     *d = &is->viddec;
    

    if (!avpkt || !avpkt->data) {
        return 0;
    }
    
    uint8_t *pkt_data = avpkt->data;
    
    if(context->refresh_session) {
        if (context->m_buffer_deep > 0 && ff_avpacket_is_idr(&context->m_buffer_packet[0], codec_id == AV_CODEC_ID_HEVC)) {
            for (int i = 0; i < context->m_buffer_deep; i++) {
                if (is->abort_request || d->queue->abort_request)
                    return -1;
                
                AVPacket* pkt = &context->m_buffer_packet[i];
                ret = decode_video_internal(context, avctx, pkt, got_picture_ptr);
                
                if(ret != 0)
                    return -1;
            }
            context->refresh_session = false;
        } else {
            NSLog(@"refresh session fail, do not find idr frame");
        }
    }
    
    if ((!extradata || extradata_size <= 0) && ff_avpacket_is_key(avpkt)) {
        if (extradata) {
            freep((void*)&extradata);
        }
        
        int start_idx = 0;
        int ps_count = 0;
        int i = 0;
        if (codec_id == AV_CODEC_ID_H264) {
            while (i < avpkt->size - 5) {
                if ((pkt_data[i] == 0 && pkt_data[i+1] == 0 && pkt_data[i+2] == 0 && pkt_data[i+3] == 1) ||
                    (pkt_data[i] == 0 && pkt_data[i+1] == 0 && pkt_data[i+2] == 1)) {
                    int next_idx = pkt_data[i+2] == 0 ? i + 4 : i + 3;
                    if (pkt_data[next_idx] == 0x67) {  //sps
                        start_idx = i;
                        ++ps_count;
                    } else if (pkt_data[next_idx] == 0x68 && ps_count < H264_PS_COUNT) {  //pps
                        ++ps_count;
                    } else if (ps_count == H264_PS_COUNT) {
                        extradata_size = i - start_idx;
                        break;
                    }
                    i = next_idx;
                }
                ++i;
            }
        } else if (codec_id == AV_CODEC_ID_HEVC) {
            while (i < avpkt->size - 5) {
                if ((pkt_data[i] == 0 && pkt_data[i+1] == 0 && pkt_data[i+2] == 0 && pkt_data[i+3] == 1) ||
                    (pkt_data[i] == 0 && pkt_data[i+1] == 0 && pkt_data[i+2] == 1)) {
                    int next_idx = pkt_data[i+2] == 0 ? i + 4 : i + 3;
                    if (pkt_data[next_idx] == 0x40) { //vps
                        start_idx = i;
                        ++ps_count;
                    } else if (pkt_data[next_idx] == 0x42) { //sps
                        ++ps_count;
                    } else if (pkt_data[next_idx] == 0x44 && ps_count < H265_PS_COUNT) { //pps
                        ++ps_count;
                    } else if (ps_count == H265_PS_COUNT) {
                        extradata_size = i - start_idx;
                        break;
                    }
                    i = next_idx;
                }
                ++i;
            }
        } else {
            NSLog(@"cannot support codec_id: %d", codec_id);
            return 0;
        }
        
        
        if (extradata_size > 0) {
            codecpar->extradata = av_mallocz(extradata_size + AV_INPUT_BUFFER_PADDING_SIZE);
            codecpar->extradata_size = extradata_size;
            memcpy(codecpar->extradata, pkt_data+start_idx, extradata_size);
            
            if (!context->refresh_request) {
                context->vt_session = vtbsession_create(context);
                if (!context->vt_session) {
                    NSLog(@"decode_video fail, can not create videotoolbox");
                    return 0;
                }
            }
        } else {
            NSLog(@"cannot get extradata");
            return 0;
        }
        
    }
    
    
    if (ff_avpacket_is_idr(avpkt, codec_id == AV_CODEC_ID_HEVC)) {
        context->refresh_session = false;
        ResetPktBuffer(context);
    }
    DuplicatePkt(context, avpkt);

    return decode_video_internal(context, avctx, avpkt, got_picture_ptr);
}


static void dict_set_string(CFMutableDictionaryRef dict, CFStringRef key, const char * value)
{
    CFStringRef string;
    string = CFStringCreateWithCString(NULL, value, kCFStringEncodingASCII);
    CFDictionarySetValue(dict, key, string);
    CFRelease(string);
}

static void dict_set_boolean(CFMutableDictionaryRef dict, CFStringRef key, BOOL value)
{
    CFDictionarySetValue(dict, key, value ? kCFBooleanTrue: kCFBooleanFalse);
}


static void dict_set_object(CFMutableDictionaryRef dict, CFStringRef key, CFTypeRef *value)
{
    CFDictionarySetValue(dict, key, value);
}

static void dict_set_data(CFMutableDictionaryRef dict, CFStringRef key, uint8_t * value, uint64_t length)
{
    CFDataRef data;
    data = CFDataCreate(NULL, value, (CFIndex)length);
    CFDictionarySetValue(dict, key, data);
    CFRelease(data);
}

static void dict_set_i32(CFMutableDictionaryRef dict, CFStringRef key,
                         int32_t value)
{
    CFNumberRef number;
    number = CFNumberCreate(NULL, kCFNumberSInt32Type, &value);
    CFDictionarySetValue(dict, key, number);
    CFRelease(number);
}

static CMFormatDescriptionRef CreateFormatDescriptionFromCodecData(CMVideoCodecType format_id, int width, int height, const uint8_t *extradata, int extradata_size, uint32_t atom)
{
    CMFormatDescriptionRef fmt_desc = NULL;
    OSStatus status;

    CFMutableDictionaryRef par = CFDictionaryCreateMutable(NULL, 0, &kCFTypeDictionaryKeyCallBacks,&kCFTypeDictionaryValueCallBacks);
    CFMutableDictionaryRef atoms = CFDictionaryCreateMutable(NULL, 0, &kCFTypeDictionaryKeyCallBacks,&kCFTypeDictionaryValueCallBacks);
    CFMutableDictionaryRef extensions = CFDictionaryCreateMutable(NULL, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);

    /* CVPixelAspectRatio dict */
    dict_set_i32(par, CFSTR ("HorizontalSpacing"), 0);
    dict_set_i32(par, CFSTR ("VerticalSpacing"), 0);
    
    /* SampleDescriptionExtensionAtoms dict */
    switch (format_id) {
        case kCMVideoCodecType_H264:
            dict_set_data(atoms, CFSTR ("avcC"), (uint8_t *)extradata, extradata_size);
            break;
        case kCMVideoCodecType_HEVC:
            dict_set_data(atoms, CFSTR ("hvcC"), (uint8_t *)extradata, extradata_size);
            break;
        default:
            break;
    }


      /* Extensions dict */
    dict_set_string(extensions, CFSTR ("CVImageBufferChromaLocationBottomField"), "left");
    dict_set_string(extensions, CFSTR ("CVImageBufferChromaLocationTopField"), "left");
    dict_set_boolean(extensions, CFSTR("FullRangeVideo"), FALSE);
    dict_set_object(extensions, CFSTR ("CVPixelAspectRatio"), (CFTypeRef *) par);
    dict_set_object(extensions, CFSTR ("SampleDescriptionExtensionAtoms"), (CFTypeRef *) atoms);
    status = CMVideoFormatDescriptionCreate(NULL, format_id, width, height, extensions, &fmt_desc);

    CFRelease(extensions);
    CFRelease(atoms);
    CFRelease(par);

    if (status == 0)
        return fmt_desc;
    else
        return NULL;
}

void videotoolbox_sync_free(Ijk_VideoToolBox_Opaque* context)
{
    context->dealloced = true;

    while (context && context->m_queue_depth > 0) {
        SortQueuePop(context);
    }

    if (context) {
        ResetPktBuffer(context);
    }

    vtbsession_destroy(context);
    vtbformat_destroy(&context->fmt_desc);

    avcodec_parameters_free(&context->codecpar);
}


void videotoolbox_sync_wait_thread_finish(Ijk_VideoToolBox_Opaque* context)
{
    if(context->queue_frame_thread)
        SDL_WaitThread(context->queue_frame_thread, NULL);
}

int queue_frame_thread_func(void* opaque)
{
    Ijk_VideoToolBox_Opaque* context = (Ijk_VideoToolBox_Opaque*)opaque;
    FFPlayer *ffp = context->ffp;
    VideoState *is = ffp->is;
    Decoder *d = &is->viddec;
    
    do {
        QueuePicture(context);
    } while(!is->abort_request && !d->queue->abort_request);
    
    return 0;
}

void videotoolbox_sync_queue_frame(Ijk_VideoToolBox_Opaque* context)
{
    context->queue_frame_thread = SDL_CreateThreadEx(&context->_queue_frame_thread, queue_frame_thread_func, context, "videotoolbox_queue_frame_thread");
    
    if(!context->queue_frame_thread) {
        NSLog(@"videotoolbox_sync_queue_frame create queue_frame_thread failed!");
    }
}

int videotoolbox_sync_decode_frame(Ijk_VideoToolBox_Opaque* context)
{
    FFPlayer *ffp = context->ffp;
    VideoState *is = ffp->is;
    Decoder *d = &is->viddec;
    int got_frame = 0;
    do {
        int ret = -1;
        if (is->abort_request || d->queue->abort_request) {
            return -1;
        }

        if (!d->packet_pending || d->queue->serial != d->pkt_serial) {
            AVPacket pkt;
            do {
                if (d->queue->nb_packets == 0) {
                    context->is_eof = true;
                    SDL_CondSignal(d->empty_queue_cond);
                }
                
                
                ffp_video_statistic_l(ffp);
                if (ffp_packet_queue_get_or_buffering(ffp, d->queue, &pkt, &d->pkt_serial, &d->finished) < 0)
                    return -1;
                
                if (ffp_is_flush_packet(&pkt)) {
                    avcodec_flush_buffers(d->avctx);
                    context->refresh_request = true;
                    context->serial += 1;
                    d->finished = 0;
                    d->next_pts = d->start_pts;
                    d->next_pts_tb = d->start_pts_tb;
                }
            } while (ffp_is_flush_packet(&pkt) || d->queue->serial != d->pkt_serial);

            av_packet_unref(&d->pkt);
            d->pkt_temp = d->pkt = pkt;
            d->packet_pending = 1;
        }

        
        //limit m_queue_depth
        while(context->m_queue_depth >= MAX_REF_FRAMES) {
            if (is->abort_request || d->queue->abort_request) {
                return -1;
            }
            usleep(10000);
        }
        
        context->is_eof = false;
        ret = decode_video(context, d->avctx, &d->pkt_temp, &got_frame);
        if (ret < 0) {
            d->packet_pending = 0;
        } else {
            d->pkt_temp.dts =
            d->pkt_temp.pts = AV_NOPTS_VALUE;
            if (d->pkt_temp.data) {
                if (d->avctx->codec_type != AVMEDIA_TYPE_AUDIO)
                    ret = d->pkt_temp.size;
                d->pkt_temp.data += ret;
                d->pkt_temp.size -= ret;
                if (d->pkt_temp.size <= 0)
                    d->packet_pending = 0;
            } else {
                if (!got_frame) {
                    d->packet_pending = 0;
                    d->finished = d->pkt_serial;
                }
            }
        }
    } while (!got_frame && !d->finished);
    
    
    return got_frame;
}

static void vtbformat_destroy(VTBFormatDesc *fmt_desc)
{
    if (!fmt_desc || !fmt_desc->fmt_desc)
        return;

    CFRelease(fmt_desc->fmt_desc);
    fmt_desc->fmt_desc = NULL;
}

static int vtbformat_init(VTBFormatDesc *fmt_desc, AVCodecParameters *codecpar)
{
    int width           = codecpar->width;
    int height          = codecpar->height;
    int level           = codecpar->level;
    int profile         = codecpar->profile;
    int sps_level       = 0;
    int sps_profile     = 0;
    int extrasize       = codecpar->extradata_size;
    int codec           = codecpar->codec_id;
    uint8_t* extradata  = codecpar->extradata;

    bool isHevcSupported = false;
    CMVideoCodecType format_id = 0;
    

    if (width < 0 || height < 0) {
        goto fail;
    }

    if (extrasize < 7 || extradata == NULL) {
        NSLog(@"avcC or hvcC atom data too small or missing");
        goto fail;
    }

    switch (codec) {
        case AV_CODEC_ID_HEVC:
            format_id = kCMVideoCodecType_HEVC;
            if (@available(iOS 11.0, *)) {
                isHevcSupported = VTIsHardwareDecodeSupported(kCMVideoCodecType_HEVC);
            } else {
                // Fallback on earlier versions
                isHevcSupported = false;
            }
            if (!isHevcSupported) {
                goto fail;
            }
            break;
            
        case AV_CODEC_ID_H264:
            format_id = kCMVideoCodecType_H264;
            break;
            
        default:
            goto fail;
    }
    
    if (extradata[0] == 1) 
    {
        if (level == 0 && sps_level > 0)
            level = sps_level;

                if (profile == 0 && sps_profile > 0)
                    profile = sps_profile;
                if (profile == FF_PROFILE_H264_MAIN && level == 32 && fmt_desc->max_ref_frames > 4) {
                    NSLog(@"%s - Main@L3.2 detected, VTB cannot decode with %d ref frames", __FUNCTION__, fmt_desc->max_ref_frames);
                    goto fail;
                }

                if (extradata[4] == 0xFE) {
                    extradata[4] = 0xFF;
                    fmt_desc->convert_3byteTo4byteNALSize = true;
                }

        fmt_desc->fmt_desc = CreateFormatDescriptionFromCodecData(format_id, width, height, extradata, extrasize,  IJK_VTB_FCC_AVCC);
        if (fmt_desc->fmt_desc == NULL) {
            goto fail;
        }

        NSLog(@"%s - using avcC atom of size(%d), ref_frames(%d)", __FUNCTION__, extrasize, fmt_desc->max_ref_frames);
    } 
    else 
    {
        if ((extradata[0] == 0 && extradata[1] == 0 && extradata[2] == 0 && extradata[3] == 1) ||
            (extradata[0] == 0 && extradata[1] == 0 && extradata[2] == 1)) 
        {
            AVIOContext *pb;
            if (avio_open_dyn_buf(&pb) < 0) {
                goto fail;
            }

            fmt_desc->convert_bytestream = true;
            if (codec == AV_CODEC_ID_HEVC) {
                ff_isom_write_hvcc(pb, extradata, extrasize, 0);
            } else {
                ff_isom_write_avcc(pb, extradata, extrasize);
            }
            
            extradata = NULL;

            extrasize = avio_close_dyn_buf(pb, &extradata);


            if (codec == AV_CODEC_ID_H264 && !validate_avcC_spc(extradata, extrasize, &fmt_desc->max_ref_frames, &sps_level, &sps_profile)) {
                av_free(extradata);
                goto fail;
            } else if (codec == AV_CODEC_ID_HEVC) {
                fmt_desc->max_ref_frames = 5;
            }

            fmt_desc->fmt_desc = CreateFormatDescriptionFromCodecData(format_id, width, height, extradata, extrasize, IJK_VTB_FCC_AVCC);
            if (fmt_desc->fmt_desc == NULL) {
                goto fail;
            }

            av_free(extradata);
        } 
        else 
        {
            NSLog(@"%s - invalid avcC atom data", __FUNCTION__);
            goto fail;
        }
    }

    fmt_desc->max_ref_frames = FFMAX(fmt_desc->max_ref_frames, 2);
    fmt_desc->max_ref_frames = FFMIN(fmt_desc->max_ref_frames, 5);

    ALOGI("m_max_ref_frames %d \n", fmt_desc->max_ref_frames);

    return 0;
fail:
    vtbformat_destroy(fmt_desc);
    return -1;
}

Ijk_VideoToolBox_Opaque* videotoolbox_sync_create(FFPlayer* ffp, AVCodecContext* avctx)
{
    int ret = 0;

    Ijk_VideoToolBox_Opaque *context_vtb = (Ijk_VideoToolBox_Opaque *)mallocz(sizeof(Ijk_VideoToolBox_Opaque));

    if (!context_vtb) {
        goto fail;
    }

    context_vtb->codecpar = avcodec_parameters_alloc();
    if (!context_vtb->codecpar)
        goto fail;

    ret = avcodec_parameters_from_context(context_vtb->codecpar, avctx);
    if (ret)
        goto fail;

    context_vtb->ffp = ffp;
    
    context_vtb->m_sort_queue = 0;
    context_vtb->m_queue_depth = 0;

    SDL_SpeedSamplerReset(&context_vtb->sampler);
    return context_vtb;

fail:
    videotoolbox_sync_free(context_vtb);
    return NULL;
}
