/**************************************************

**** @file:     ijkplayer_delegate_def.h

**** @brief:

**** @author:   kiddpeng

**** @date:     2024/11/01

**** @group:    PixVideo

**** @copyright: Copyright 2024 PixVideo. All Rights Reserved.

***************************************************/

#ifndef IJKPLAYER_DELEGATE_DEF_H
#define IJKPLAYER_DELEGATE_DEF_H

#ifdef _WIN32
#define EXPORT_API  __declspec(dllexport)
#else
#define EXPORT_API  __attribute__ ((visibility ("default")))
#endif

#include <stdint.h>
#include "ijkplayer_internal.h"
#include "ff_ffmsg_queue.h"
#include "ff_ffpipeline.h"
#include "pixvideo/pandora_player.h"

#ifndef PIXVIDEO_CORE_VERSION
#ifdef PIXVIDEO_PLAYER_VERSION
#define PIXVIDEO_CORE_VERSION PIXVIDEO_PLAYER_VERSION
#else
#error "PIXVIDEO_PLAYER_VERSION is not defined"
#endif
#endif

typedef void     (*pixvideo_network_data_delegate)(const unsigned char* data, unsigned int size, int tag);
typedef void*    (*pixvideo_file_open_delegate)(const char* path, int mode);
typedef int      (*pixvideo_file_read_delegate)(void* file_handle, unsigned char *buf, int size);
typedef int64_t  (*pixvideo_file_seek_delegate)(void* file_handle, int64_t offset, int whence);
typedef int      (*pixvideo_file_close_delegate)(void* file_handle);
typedef void     (*pixvideo_alpha_info_delegate)(void *formatContext, void *ffPlayer);
typedef IJKFF_Pipenode* (*pixvideo_audiotoolbox_delegate)(struct FFPlayer *ffp); // create audio decoder from audiotoolbox

EXPORT_API void pixvideo_set_global_delgate(pixvideo_network_data_delegate network_data_del,
                                    pixvideo_file_open_delegate  h5_file_open_del,
                                    pixvideo_file_read_delegate  h5_file_read_del,
                                    pixvideo_file_seek_delegate  h5_file_seek_del,
                                    pixvideo_file_close_delegate h5_file_close_del,
                                    pixvideo_file_open_delegate  pandora_file_open_del,
                                    pixvideo_file_read_delegate  pandora_file_read_del,
                                    pixvideo_file_seek_delegate  pandora_file_seek_del,
                                    pixvideo_file_close_delegate pandora_file_close_del,
                                    pixvideo_audiotoolbox_delegate audiotoolbox_del,
                                    pixvideo_alpha_info_delegate alpah_info_del);

#if __ANDROID__
EXPORT_API SDL_Vout *pixvideo_SDL_VoutAndroid_CreateForNull();
#endif

#if TARGET_OS_MAC
EXPORT_API IjkMediaPlayer *pixvideo_ijkmp_mac_create(int (*msg_loop)(void*));
#endif

#if TARGET_OS_IPHONE
EXPORT_API IjkMediaPlayer *pixvideo_ijkmp_ios_create(int (*msg_loop)(void*));
EXPORT_API void pixvideo_ijkmp_ios_set_metal_device(void *device);
#endif

EXPORT_API int  pixvideo_get_log_level();
EXPORT_API int pixvideo_SDL_AoutGetAudioSessionId(SDL_Aout *aout);
EXPORT_API int pixvideo_SDL_LockMutex(SDL_mutex *mutex);
EXPORT_API int pixvideo_SDL_CondSignal(SDL_cond *cond);
EXPORT_API int pixvideo_SDL_UnlockMutex(SDL_mutex *mutex);
EXPORT_API void pixvideo_SDL_DestroyMutexP(SDL_mutex **mutex);
EXPORT_API void pixvideo_SDL_DestroyCondP(SDL_cond **cond);
EXPORT_API int pixvideo_SDL_Android_GetApiLevel();
EXPORT_API int pixvideo_SDL_CondWaitTimeout(SDL_cond *cond, SDL_mutex *mutex, uint32_t ms);
EXPORT_API void pixvideo_SDL_SpeedSamplerReset(SDL_SpeedSampler *sampler);
EXPORT_API float pixvideo_SDL_SpeedSamplerAdd(SDL_SpeedSampler *sampler, int enable_log, const char *log_tag);
EXPORT_API SDL_Aout *pixvideo_SDL_AoutAndroid_CreateForOpenSLES();
EXPORT_API SDL_mutex *pixvideo_SDL_CreateMutex(void);
EXPORT_API SDL_cond *pixvideo_SDL_CreateCond(void);
EXPORT_API SDL_Aout *pixvideo_SDL_AoutAndroid_CreateForAudioTrack();
EXPORT_API void pixvideo_SDL_Aout_FreeInternal(SDL_Aout *aout);
EXPORT_API SDL_Aout *pixvideo_SDL_Aout_CreateInternal(size_t opaque_size);
EXPORT_API SDL_Vout *pixvideo_SDL_Vout_CreateInternal(size_t opaque_size);
EXPORT_API SDL_VoutOverlay *pixvideo_SDL_VoutFFmpeg_CreateOverlay(int width, int height, int frame_format, SDL_Vout *vout);
#if __ANDROID__ || OHOS
EXPORT_API SDL_VoutOverlay *pixvideo_SDL_VoutAMediaCodec_CreateOverlay(int width, int height, SDL_Vout *vout);
#endif
EXPORT_API void pixvideo_SDL_Vout_FreeInternal(SDL_Vout *aout);
EXPORT_API void pixvideo_SDL_WaitThread(SDL_Thread *thread, int *status);
EXPORT_API int64_t pixvideo_SDL_GetCurrentTime();
EXPORT_API SDL_Thread *pixvideo_SDL_CreateThreadEx(SDL_Thread *thread, int (*fn)(void *), void *data, const char *name);
EXPORT_API void pixvideo_SDL_DetachThread(SDL_Thread *thread);
EXPORT_API void pixvideo_ffp_notify_msg1(FFPlayer *ffp, int what);
EXPORT_API void pixvideo_ffp_notify_msg2(FFPlayer *ffp, int what, int arg1);
EXPORT_API void pixvideo_ffp_notify_msg3(FFPlayer *ffp, int what, int arg1, int arg2);
EXPORT_API void pixvideo_ffp_notify_msg4(FFPlayer *ffp, int what, int arg1, int arg2, void *obj, int obj_len);
EXPORT_API void pixvideo_ffp_packet_queue_abort(PacketQueue *q);
EXPORT_API IJKFF_Pipeline *pixvideo_ffpipeline_alloc(SDL_Class *opaque_class, size_t opaque_size);
EXPORT_API IJKFF_Pipenode *pixvideo_ffpipenode_create_video_decoder_from_ffplay(struct FFPlayer *ffp);
EXPORT_API IJKFF_Pipenode *pixvideo_ffpipenode_create_audio_decoder_from_ffplay(FFPlayer *ffp);
EXPORT_API IJKFF_Pipenode *pixvideo_ffpipenode_alloc(size_t opaque_size);
EXPORT_API void pixvideo_ffpipenode_free_p(IJKFF_Pipenode **node);
EXPORT_API void pixvideo_ffp_set_video_codec_info(FFPlayer *ffp, const char *module, const char *codec);
EXPORT_API int pixvideo_ffp_queue_picture(FFPlayer *ffp, AVFrame *src_frame, double pts, double duration, int64_t pos, int serial);
EXPORT_API int pixvideo_ffp_get_master_sync_type(VideoState *is);
EXPORT_API double pixvideo_ffp_get_master_clock(VideoState *is);
EXPORT_API int pixvideo_ffp_video_thread(FFPlayer *ffp);
EXPORT_API int pixvideo_ffp_packet_queue_get_or_buffering(FFPlayer *ffp, PacketQueue *q, AVPacket *pkt, int *serial, int *finished);
EXPORT_API bool pixvideo_ffp_is_flush_packet(AVPacket *pkt);
EXPORT_API void pixvideo_ffp_set_soft_subtitle_path(FFPlayer *ffp, const char *path);
EXPORT_API void pixvideo_ffp_close_soft_subtitle(FFPlayer *ffp);
EXPORT_API int pixvideo_ffp_queue_audio_frame(FFPlayer *ffp, AVFrame *frame);
EXPORT_API int pixvideo_ffp_set_decryption_key(FFPlayer *ffp, const char* decryption_key);
EXPORT_API void pixvideo_msg_queue_init(MessageQueue* q);
EXPORT_API void pixvideo_msg_queue_start(MessageQueue* q);
EXPORT_API int pixvideo_msg_queue_put(MessageQueue *q, AVMessage *msg);
EXPORT_API void pixvideo_msg_queue_abort(MessageQueue *q);
EXPORT_API void pixvideo_msg_queue_destroy(MessageQueue *q);
EXPORT_API int pixvideo_msg_queue_get(MessageQueue *q, AVMessage *msg, int block);
EXPORT_API void pixvideo_msg_queue_put_simple1(MessageQueue *q, int what);
EXPORT_API void pixvideo_msg_queue_put_simple2(MessageQueue *q, int what, int arg1);
EXPORT_API void pixvideo_msg_queue_put_simple3(MessageQueue *q, int what, int arg1, int arg2);
EXPORT_API void pixvideo_msg_queue_put_simple4(MessageQueue *q, int what, int arg1, int arg2, void *obj, int obj_len);
EXPORT_API void pixvideo_msg_queue_put_simple5(MessageQueue *q, int what, int arg1, int arg2, void *weak_obj);
EXPORT_API void pixvideo_msg_queue_put_string4(MessageQueue* q, int what, int arg1, int arg2, void* obj, int obj_len);
EXPORT_API void pixvideo_msg_queue_clear(MessageQueue* q);
EXPORT_API IjkMediaPlayer *pixvideo_ijkmp_create(int (*msg_loop)(void*));
EXPORT_API void pixvideo_ijkmp_set_option(IjkMediaPlayer *mp, int opt_category, const char *name, const char *value);
EXPORT_API void pixvideo_ijkmp_set_option_int(IjkMediaPlayer *mp, int opt_category, const char *name, int64_t value);
EXPORT_API void pixvideo_ijkmp_set_frame_callback(IjkMediaPlayer *mp, ijkmp_frame_callback callback);
EXPORT_API void pixvideo_ijkmp_set_video_end_callback(IjkMediaPlayer *mp, ijkmp_video_end_callback callback);
EXPORT_API void pixvideo_ijkmp_global_set_log_level(int log_level);
EXPORT_API void pixvideo_ijkmp_set_loop(IjkMediaPlayer *mp, int loop);
EXPORT_API int pixvideo_ijkmp_set_data_source(IjkMediaPlayer *mp, const char *url);
EXPORT_API int pixvideo_ijkmp_prepare_async(IjkMediaPlayer *mp);
EXPORT_API int pixvideo_ijkmp_get_msg(IjkMediaPlayer *mp, AVMessage *msg, int block);
EXPORT_API int pixvideo_ijkmp_pause(IjkMediaPlayer *mp);
EXPORT_API void pixvideo_ijkmp_set_volume(IjkMediaPlayer *mp, float left, float right);
EXPORT_API int pixvideo_ijkmp_seek_to(IjkMediaPlayer *mp, long msec);
EXPORT_API long pixvideo_ijkmp_get_duration(IjkMediaPlayer *mp);
EXPORT_API long pixvideo_ijkmp_get_current_position(IjkMediaPlayer *mp);
EXPORT_API void pixvideo_ijkmp_set_playback_rate(IjkMediaPlayer *mp, float rate);
EXPORT_API int pixvideo_ijkmp_stop(IjkMediaPlayer *mp);
EXPORT_API void pixvideo_ijkmp_destroy(IjkMediaPlayer *mp);
EXPORT_API int pixvideo_ijkmp_start(IjkMediaPlayer *mp);
EXPORT_API void pixvideo_ijkmp_global_init();
EXPORT_API void pixvideo_ijkmp_copy_player_properties(IjkMediaPlayer *src, IjkMediaPlayer *dst);
EXPORT_API double pixvideo_ijkmp_get_master_clock(IjkMediaPlayer *mp);
EXPORT_API int pixvideo_ijkmp_get_loop(IjkMediaPlayer *mp);
EXPORT_API void pixvideo_ijkmp_release_nativewindow(IjkMediaPlayer *mp);
EXPORT_API void pixvideo_ijkmp_enable_seek_and_pause(IjkMediaPlayer *mp);
EXPORT_API void pixvideo_ijkmp_step_to_next_frame(IjkMediaPlayer *mp);
EXPORT_API IjkIOManagerContext* pixvideo_ijkcache_init_context(const char* url, const char* cache_file_path, const char* cache_map_path);
EXPORT_API void pixvideo_ijkcache_destroy_context(IjkIOManagerContext *h);
EXPORT_API int pixvideo_ijkcache_is_cache_complete(IjkIOManagerContext *h);
EXPORT_API int pixvideo_ijkcache_save_to_mp4(IjkIOManagerContext *h, const char* cache_file_path, const char* output_path);
EXPORT_API void pixvideo_ijkmp_cancel_seek(IjkMediaPlayer *mp);
EXPORT_API void PixVideoCopyPlane(const uint8_t* src_y,
               int src_stride_y,
               uint8_t* dst_y,
               int dst_stride_y,
               int width,
               int height);

EXPORT_API void PixVideoCopyPlaneUV(const uint8_t* src_u,
               int src_stride_u,
               const uint8_t* src_v,
               int src_stride_v,
               uint8_t* dst,
               int width,
               int height);

EXPORT_API void PixVideoSplitUVPlane(const uint8_t* src_uv,
                  int src_stride_uv,
                  uint8_t* dst_u,
                  int dst_stride_u,
                  uint8_t* dst_v,
                  int dst_stride_v,
                  int width,
                  int height);

EXPORT_API int PixVideoI420ToABGR(const uint8_t* src_y,
               int src_stride_y,
               const uint8_t* src_u,
               int src_stride_u,
               const uint8_t* src_v,
               int src_stride_v,
               uint8_t* dst_abgr,
               int dst_stride_abgr,
               int width,
               int height);

EXPORT_API int PixVideoI420ToRAW(const uint8_t* src_y,
              int src_stride_y,
              const uint8_t* src_u,
              int src_stride_u,
              const uint8_t* src_v,
              int src_stride_v,
              uint8_t* dst_raw,
              int dst_stride_raw,
              int width,
              int height);

EXPORT_API int PixVideoNV12ToABGR(const uint8_t* src_y,
               int src_stride_y,
               const uint8_t* src_uv,
               int src_stride_uv,
               uint8_t* dst_abgr,
               int dst_stride_abgr,
               int width,
               int height);

EXPORT_API int PixVideoNV12ToRAW(const uint8_t* src_y,
              int src_stride_y,
              const uint8_t* src_uv,
              int src_stride_uv,
              uint8_t* dst_raw,
              int dst_stride_raw,
              int width,
              int height);

EXPORT_API const char* pixvideo_videocore_get_version();
#endif //IJKPLAYER_DELEGATE_DEF_H
