/**************************************************

**** @file:     ijkplayer_delegate_def.c

**** @brief:

**** @author:   kiddpeng

**** @date:     2024/11/01

**** @group:    PixVideo

**** @copyright: Copyright 2024 PixVideo. All Rights Reserved.

***************************************************/

#include "ijkplayer_delegate_def.h"
#include "ijkplayer_internal.h"
#include "libyuv/planar_functions.h"
#include <stdlib.h>

#include "ffpipenode_ffplay_vdec.h"
#include "ffpipenode_ffplay_adec.h"

#ifdef __ANDROID__
#include "android/ijksdl_android_jni.h"
#include "android/ijksdl_aout_android_opensles.h"
#include "android/ijksdl_aout_android_audiotrack.h"
#include "android/pipeline/ffpipeline_native_android.h"
#endif

#ifdef OHOS
#include "android/ijksdl_vout_overlay_android_mediacodec.h"
#endif

#include "ijksdl_aout_internal.h"
#include "ijksdl_vout_internal.h"
#include "ijksdl_vout_overlay_ffmpeg.h"
#include "ijkplayer.h"

#if TARGET_OS_IPHONE
#include "ijkplayer_ios.h"
#endif

const char* s_global_pix_video_core_version = "PixVideoCore Version: " PIXVIDEO_CORE_VERSION;

pixvideo_network_data_delegate g_network_data_del = NULL;
pixvideo_file_open_delegate g_pandora_file_open_del = NULL;
pixvideo_file_read_delegate g_pandora_file_read_del = NULL;
pixvideo_file_seek_delegate g_pandora_file_seek_del = NULL;
pixvideo_file_close_delegate g_pandora_file_close_del = NULL;
pixvideo_file_open_delegate g_h5_file_open_del = NULL;
pixvideo_file_read_delegate g_h5_file_read_del = NULL;
pixvideo_file_seek_delegate g_h5_file_seek_del = NULL;
pixvideo_file_close_delegate g_h5_file_close_del = NULL;
pixvideo_audiotoolbox_delegate g_audiotoolbox_del = NULL;
pixvideo_alpha_info_delegate g_alpah_info_del = NULL;

void pixvideo_set_global_delgate(pixvideo_network_data_delegate network_data_del,
                                    pixvideo_file_open_delegate  h5_file_open_del,
                                    pixvideo_file_read_delegate  h5_file_read_del,
                                    pixvideo_file_seek_delegate  h5_file_seek_del,
                                    pixvideo_file_close_delegate h5_file_close_del,
                                    pixvideo_file_open_delegate  pandora_file_open_del,
                                    pixvideo_file_read_delegate  pandora_file_read_del,
                                    pixvideo_file_seek_delegate  pandora_file_seek_del,
                                    pixvideo_file_close_delegate pandora_file_close_del,
                                    pixvideo_audiotoolbox_delegate audiotoolbox_del,
                                    pixvideo_alpha_info_delegate alpah_info_del)
{
    g_network_data_del = network_data_del;
    g_h5_file_open_del = h5_file_open_del;
    g_h5_file_read_del = h5_file_read_del;
    g_h5_file_seek_del = h5_file_seek_del;
    g_h5_file_close_del = h5_file_close_del;
    g_pandora_file_open_del = pandora_file_open_del;
    g_pandora_file_read_del = pandora_file_read_del;
    g_pandora_file_seek_del = pandora_file_seek_del;
    g_pandora_file_close_del = pandora_file_close_del;
    g_h5_file_close_del = h5_file_close_del;
    g_audiotoolbox_del = audiotoolbox_del;
    g_alpah_info_del = alpah_info_del;
}

#if __ANDROID__
extern SDL_Vout *SDL_VoutAndroid_CreateForNull();
SDL_Vout *pixvideo_SDL_VoutAndroid_CreateForNull()
{
    return SDL_VoutAndroid_CreateForNull();
}
#endif

#if TARGET_OS_MAC
extern IjkMediaPlayer *ijkmp_mac_create(int (*msg_loop)(void*));
IjkMediaPlayer *pixvideo_ijkmp_mac_create(int (*msg_loop)(void*))
{
    return ijkmp_mac_create(msg_loop);
}
#endif


#if TARGET_OS_IPHONE
IjkMediaPlayer *pixvideo_ijkmp_ios_create(int (*msg_loop)(void*))
{
    return ijkmp_ios_create(msg_loop);
}

void pixvideo_ijkmp_ios_set_metal_device(void *device)
{
    ijkmp_ios_set_metal_device(device);
}
#endif


void pixvideo_ijkmp_set_option(IjkMediaPlayer *mp, int opt_category, const char *name, const char *value)
{
    ijkmp_set_option(mp, opt_category, name, value);
}

void pixvideo_ijkmp_set_option_int(IjkMediaPlayer *mp, int opt_category, const char *name, int64_t value)
{
    ijkmp_set_option_int(mp, opt_category, name, value);
}

void pixvideo_ijkmp_set_frame_callback(IjkMediaPlayer *mp, ijkmp_frame_callback callback)
{
    ijkmp_set_frame_callback(mp, callback);
}

void pixvideo_ijkmp_set_video_end_callback(IjkMediaPlayer *mp, ijkmp_video_end_callback callback)
{
    ijkmp_set_video_end_callback(mp, callback);
}

void pixvideo_ijkmp_global_init()
{
    ijkmp_global_init();
}

void pixvideo_ijkmp_set_loop(IjkMediaPlayer *mp, int loop)
{
    ijkmp_set_loop(mp, loop);
}

int pixvideo_ijkmp_set_data_source(IjkMediaPlayer *mp, const char *url)
{
    return ijkmp_set_data_source(mp, url);
}

int pixvideo_ijkmp_prepare_async(IjkMediaPlayer *mp)
{
    return ijkmp_prepare_async(mp);
}

int pixvideo_ijkmp_get_msg(IjkMediaPlayer *mp, AVMessage *msg, int block)
{
    return ijkmp_get_msg(mp, msg, block);
}

void PixVideoCopyPlane(const uint8_t* src_y,
               int src_stride_y,
               uint8_t* dst_y,
               int dst_stride_y,
               int width,
               int height)
{
    CopyPlane(src_y, src_stride_y, dst_y, dst_stride_y, width, height);
}

void PixVideoCopyPlaneUV(const uint8_t* src_u,
               int src_stride_u,
               const uint8_t* src_v,
               int src_stride_v,
               uint8_t* dst,
               int width,
               int height)
{
    CopyPlaneUV(src_u, src_stride_u, src_v, src_stride_v, dst, width, height);
}

int pixvideo_ijkmp_pause(IjkMediaPlayer *mp)
{
    return ijkmp_pause(mp);
}

void pixvideo_ijkmp_set_volume(IjkMediaPlayer *mp, float left, float right)
{
    ijkmp_set_volume(mp, left, right);
}

int pixvideo_ijkmp_seek_to(IjkMediaPlayer *mp, long msec)
{
    return ijkmp_seek_to(mp, msec);
}

long pixvideo_ijkmp_get_duration(IjkMediaPlayer *mp)
{
    return ijkmp_get_duration(mp);
}

long pixvideo_ijkmp_get_current_position(IjkMediaPlayer *mp)
{
    return ijkmp_get_current_position(mp);
}

void pixvideo_ijkmp_set_playback_rate(IjkMediaPlayer *mp, float rate)
{
    ijkmp_set_playback_rate(mp, rate);
}

int pixvideo_ijkmp_stop(IjkMediaPlayer *mp)
{
    return ijkmp_stop(mp);
}

void pixvideo_ijkmp_destroy(IjkMediaPlayer *mp)
{
    ijkmp_destroy(mp);
}

int pixvideo_ijkmp_start(IjkMediaPlayer *mp)
{
    return ijkmp_start(mp);
}

void pixvideo_ijkmp_cancel_seek(IjkMediaPlayer *mp)
{
    ijkmp_cancel_seek(mp);
}

void pixvideo_msg_queue_init(MessageQueue* q)
{
    msg_queue_init(q);
}

void pixvideo_msg_queue_start(MessageQueue* q)
{
    msg_queue_start(q);
}

int pixvideo_msg_queue_put(MessageQueue *q, AVMessage *msg)
{
    return msg_queue_put(q, msg);
}

void pixvideo_msg_queue_put_simple1(MessageQueue *q, int what)
{
    msg_queue_put_simple1(q, what);
}

void pixvideo_msg_queue_put_simple2(MessageQueue *q, int what, int arg1)
{
    msg_queue_put_simple2(q, what, arg1);
}

void pixvideo_msg_queue_put_simple3(MessageQueue *q, int what, int arg1, int arg2)
{
    msg_queue_put_simple3(q, what, arg1, arg2);
}

void pixvideo_msg_queue_put_simple4(MessageQueue *q, int what, int arg1, int arg2, void *obj, int obj_len)
{
    msg_queue_put_simple4(q, what, arg1, arg2, obj, obj_len);
}

void pixvideo_msg_queue_put_simple5(MessageQueue *q, int what, int arg1, int arg2, void *weak_obj)
{
    msg_queue_put_simple5(q, what, arg1, arg2, weak_obj);
}

void pixvideo_msg_queue_put_string4(MessageQueue* q, int what, int arg1, int arg2, void* obj, int obj_len)
{
    msg_queue_put_string4(q, what, arg1, arg2, obj, obj_len);
}

void pixvideo_msg_queue_abort(MessageQueue *q)
{
    msg_queue_abort(q);
}

void pixvideo_msg_queue_destroy(MessageQueue *q)
{
    msg_queue_destroy(q);
}

int pixvideo_msg_queue_get(MessageQueue *q, AVMessage *msg, int block)
{
    return msg_queue_get(q, msg, block);
}

void pixvideo_msg_queue_clear(MessageQueue* q)
{
    msg_queue_clear(q);
}

void pixvideo_ijkmp_global_set_log_level(int log_level)
{
    ijkmp_global_set_log_level(log_level);
}

int64_t pixvideo_SDL_GetCurrentTime()
{
    return SDL_GetCurrentTime();
}

void pixvideo_ijkmp_copy_player_properties(IjkMediaPlayer *src, IjkMediaPlayer *dst)
{
    ijkmp_copy_player_properties(src, dst);
}

double pixvideo_ijkmp_get_master_clock(IjkMediaPlayer *mp)
{
    return ijkmp_get_master_clock(mp);
}

int pixvideo_ijkmp_get_loop(IjkMediaPlayer *mp)
{
    return ijkmp_get_loop(mp);
}

void pixvideo_ijkmp_release_nativewindow(IjkMediaPlayer *mp)
{
    ijkmp_release_nativewindow(mp);
}

#ifdef __ANDROID__
SDL_Aout *pixvideo_SDL_AoutAndroid_CreateForOpenSLES()
{
    return SDL_AoutAndroid_CreateForOpenSLES();
}

int pixvideo_SDL_Android_GetApiLevel()
{
    return SDL_Android_GetApiLevel();
}

SDL_Aout *pixvideo_SDL_AoutAndroid_CreateForAudioTrack()
{
    return SDL_AoutAndroid_CreateForAudioTrack();
}
#endif

extern IjkIOManagerContext* ijkcache_init_context(const char* url, const char* cache_file_path, const char* cache_map_path);

IjkIOManagerContext* pixvideo_ijkcache_init_context(const char* url, const char* cache_file_path, const char* cache_map_path)
{
    return ijkcache_init_context(url, cache_file_path, cache_map_path);
}

extern void ijkcache_destroy_context(IjkIOManagerContext *h);
extern int ijkcache_is_cache_complete(IjkIOManagerContext *h);
extern int ijkcache_save_to_mp4(IjkIOManagerContext *h, const char* cache_file_path, const char* output_path);

void pixvideo_ijkcache_destroy_context(IjkIOManagerContext *h)
{
    ijkcache_destroy_context(h);
}

SDL_Thread *pixvideo_SDL_CreateThreadEx(SDL_Thread *thread, int (*fn)(void *), void *data, const char *name)
{
    return SDL_CreateThreadEx(thread, fn, data, name);
}

void pixvideo_SDL_DetachThread(SDL_Thread *thread)
{
    SDL_DetachThread(thread);
}

void pixvideo_ijkmp_enable_seek_and_pause(IjkMediaPlayer *mp)
{
    ijkmp_enable_seek_and_pause(mp);
}

void pixvideo_ijkmp_step_to_next_frame(IjkMediaPlayer *mp)
{
    ijkmp_step_to_next_frame(mp);
}

int pixvideo_ijkcache_is_cache_complete(IjkIOManagerContext *h)
{
    return ijkcache_is_cache_complete(h);
}

int pixvideo_ijkcache_save_to_mp4(IjkIOManagerContext *h, const char* cache_file_path, const char* output_path)
{
    return ijkcache_save_to_mp4(h, cache_file_path, output_path);
}

void PixVideoSplitUVPlane(const uint8_t* src_uv,
                  int src_stride_uv,
                  uint8_t* dst_u,
                  int dst_stride_u,
                  uint8_t* dst_v,
                  int dst_stride_v,
                  int width,
                  int height)
{
    SplitUVPlane(src_uv, src_stride_uv, dst_u, dst_stride_u, dst_v, dst_stride_v, width, height);
}

int PixVideoI420ToABGR(const uint8_t* src_y,
               int src_stride_y,
               const uint8_t* src_u,
               int src_stride_u,
               const uint8_t* src_v,
               int src_stride_v,
               uint8_t* dst_abgr,
               int dst_stride_abgr,
               int width,
               int height)
{
    return I420ToABGR(src_y, src_stride_y, src_u, src_stride_u, src_v, src_stride_v, dst_abgr, dst_stride_abgr, width, height);
}

int PixVideoI420ToRAW(const uint8_t* src_y,
              int src_stride_y,
              const uint8_t* src_u,
              int src_stride_u,
              const uint8_t* src_v,
              int src_stride_v,
              uint8_t* dst_raw,
              int dst_stride_raw,
              int width,
              int height)
{
    return I420ToRAW(src_y, src_stride_y, src_u, src_stride_u, src_v, src_stride_v, dst_raw, dst_stride_raw, width, height);
}

int PixVideoNV12ToABGR(const uint8_t* src_y,
               int src_stride_y,
               const uint8_t* src_uv,
               int src_stride_uv,
               uint8_t* dst_abgr,
               int dst_stride_abgr,
               int width,
               int height)
{
    return NV12ToABGR(src_y, src_stride_y, src_uv, src_stride_uv, dst_abgr, dst_stride_abgr, width, height);
}

int PixVideoNV12ToRAW(const uint8_t* src_y,
              int src_stride_y,
              const uint8_t* src_uv,
              int src_stride_uv,
              uint8_t* dst_raw,
              int dst_stride_raw,
              int width,
              int height)
{
    return NV12ToRAW(src_y, src_stride_y, src_uv, src_stride_uv, dst_raw, dst_stride_raw, width, height);
}

int pixvideo_SDL_LockMutex(SDL_mutex *mutex)
{
    return SDL_LockMutex(mutex);
}

int pixvideo_SDL_CondSignal(SDL_cond *cond)
{
    return SDL_CondSignal(cond);
}

int pixvideo_SDL_UnlockMutex(SDL_mutex *mutex)
{
    return SDL_UnlockMutex(mutex);
}

SDL_mutex *pixvideo_SDL_CreateMutex()
{
    return SDL_CreateMutex();
}

SDL_cond *pixvideo_SDL_CreateCond(void)
{
    return SDL_CreateCond();
}

void pixvideo_SDL_DestroyMutexP(SDL_mutex **mutex)
{
    SDL_DestroyMutexP(mutex);
}

void pixvideo_SDL_DestroyCondP(SDL_cond **cond)
{
    SDL_DestroyCondP(cond);
}

int pixvideo_SDL_CondWaitTimeout(SDL_cond *cond, SDL_mutex *mutex, uint32_t ms)
{
    return SDL_CondWaitTimeout(cond, mutex, ms);
}

void pixvideo_ffp_notify_msg1(FFPlayer *ffp, int what)
{
    ffp_notify_msg1(ffp, what);
}

void pixvideo_ffp_notify_msg2(FFPlayer *ffp, int what, int arg1)
{
    ffp_notify_msg2(ffp, what, arg1);
}

void pixvideo_ffp_notify_msg3(FFPlayer *ffp, int what, int arg1, int arg2)
{
    ffp_notify_msg3(ffp, what, arg1, arg2);
}

void pixvideo_ffp_notify_msg4(FFPlayer *ffp, int what, int arg1, int arg2, void *obj, int obj_len)
{
    ffp_notify_msg4(ffp, what, arg1, arg2, obj, obj_len);
}

void pixvideo_ffp_packet_queue_abort(PacketQueue *q)
{
    ffp_packet_queue_abort(q);
}

IJKFF_Pipeline *pixvideo_ffpipeline_alloc(SDL_Class *opaque_class, size_t opaque_size)
{
    return ffpipeline_alloc(opaque_class, opaque_size);
}

IJKFF_Pipenode *pixvideo_ffpipenode_create_video_decoder_from_ffplay(struct FFPlayer *ffp)
{
    return ffpipenode_create_video_decoder_from_ffplay(ffp);
}


IJKFF_Pipenode *pixvideo_ffpipenode_create_audio_decoder_from_ffplay(FFPlayer *ffp)
{
    return ffpipenode_create_audio_decoder_from_ffplay(ffp);
}

IJKFF_Pipenode *pixvideo_ffpipenode_alloc(size_t opaque_size)
{
    return ffpipenode_alloc(opaque_size);
}

void pixvideo_ffpipenode_free_p(IJKFF_Pipenode **node)
{
    ffpipenode_free_p(node);
}

void pixvideo_ffp_set_video_codec_info(FFPlayer *ffp, const char *module, const char *codec)
{
    ffp_set_video_codec_info(ffp, module, codec);
}

void pixvideo_SDL_SpeedSamplerReset(SDL_SpeedSampler *sampler)
{
    SDL_SpeedSamplerReset(sampler);
}

int pixvideo_ffp_queue_picture(FFPlayer *ffp, AVFrame *src_frame, double pts, double duration, int64_t pos, int serial)
{
    return ffp_queue_picture(ffp, src_frame, pts, duration, pos, serial);
}

int pixvideo_ffp_get_master_sync_type(VideoState *is)
{
    return ffp_get_master_sync_type(is);
}

double pixvideo_ffp_get_master_clock(VideoState *is)
{
    return ffp_get_master_clock(is);
}

float pixvideo_SDL_SpeedSamplerAdd(SDL_SpeedSampler *sampler, int enable_log, const char *log_tag)
{
    return SDL_SpeedSamplerAdd(sampler, enable_log, log_tag);
}

int pixvideo_ffp_video_thread(FFPlayer *ffp)
{
    return ffp_video_thread(ffp);
}

void pixvideo_SDL_WaitThread(SDL_Thread *thread, int *status)
{
    SDL_WaitThread(thread, status);
}

int pixvideo_ffp_packet_queue_get_or_buffering(FFPlayer *ffp, PacketQueue *q, AVPacket *pkt, int *serial, int *finished)
{
    return ffp_packet_queue_get_or_buffering(ffp, q, pkt, serial, finished);
}

bool pixvideo_ffp_is_flush_packet(AVPacket *pkt)
{
    return ffp_is_flush_packet(pkt);
}

void pixvideo_ffp_set_soft_subtitle_path(FFPlayer *ffp, const char *path)
{
    ffp_set_soft_subtitle_path(ffp, path);
}

void pixvideo_ffp_close_soft_subtitle(FFPlayer *ffp)
{
    ffp_close_soft_subtitle(ffp);
}

int pixvideo_ffp_queue_audio_frame(FFPlayer *ffp, AVFrame *frame)
{
    return ffp_queue_audio_frame(ffp, frame);
}

int pixvideo_ffp_set_decryption_key(FFPlayer *ffp, const char* decryption_key)
{
    return ffp_set_decryption_key(ffp, decryption_key);
}

int pixvideo_SDL_AoutGetAudioSessionId(SDL_Aout *aout)
{
    return SDL_AoutGetAudioSessionId(aout);
}

void pixvideo_SDL_Aout_FreeInternal(SDL_Aout *aout)
{
    SDL_Aout_FreeInternal(aout);
}

void pixvideo_SDL_Vout_FreeInternal(SDL_Vout *aout)
{
    SDL_Vout_FreeInternal(aout);
}

SDL_Aout *pixvideo_SDL_Aout_CreateInternal(size_t opaque_size)
{
    return SDL_Aout_CreateInternal(opaque_size);
}

SDL_Vout *pixvideo_SDL_Vout_CreateInternal(size_t opaque_size)
{
    return SDL_Vout_CreateInternal(opaque_size); 
}

SDL_VoutOverlay *pixvideo_SDL_VoutFFmpeg_CreateOverlay(int width, int height, int frame_format, SDL_Vout *vout)
{
    return SDL_VoutFFmpeg_CreateOverlay(width, height, frame_format, vout);
}

#if __ANDROID__ || OHOS
SDL_VoutOverlay *pixvideo_SDL_VoutAMediaCodec_CreateOverlay(int width, int height, SDL_Vout *vout)
{
    return SDL_VoutAMediaCodec_CreateOverlay(width, height, vout);
}
#endif

IjkMediaPlayer *pixvideo_ijkmp_create(int (*msg_loop)(void*))
{
    return ijkmp_create(msg_loop);
}

//version
const char* pixvideo_videocore_get_version()
{
    return s_global_pix_video_core_version;
}