/*
 * ff_ffmsg.h
 *      based on PacketQueue in ffplay.c
 *
 * Copyright (c) 2013 Bilibili
 * Copyright (c) 2013 Zhang Rui <bbcallen@gmail.com>
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

#ifndef FFPLAY__FF_FFMSG_H
#define FFPLAY__FF_FFMSG_H

#define FFP_MSG_FLUSH                       0
#define FFP_MSG_ERROR                       100     /* arg1 = error */
#define FFP_MSG_PREPARED                    200
#define FFP_MSG_SWITCH_CLARITY_PREPARED     201
#define FFP_MSG_SWITCH_CLARITY_START        202
#define FFP_MSG_LIVE_PROGRESS_WARNING       210
#define FFP_MSG_COMPLETED                   300
#define FFP_MSG_LOOP_COMPLETED              301 
#define FFP_MSG_PROGRESS                    310
#define FFP_MSG_VIDEO_SIZE_CHANGED          400     /* arg1 = width, arg2 = height */
#define FFP_MSG_VIDEO_RENDERING_START       402
#define FFP_MSG_AUDIO_RENDERING_START       403
#define FFP_MSG_AUDIO_DECODED_START         405
#define FFP_MSG_VIDEO_DECODED_START         406
#define FFP_MSG_OPEN_INPUT                  407
#define FFP_MSG_FIND_STREAM_INFO            408
#define FFP_MSG_COMPONENT_OPEN              409
#define FFP_MSG_VIDEO_SEEK_RENDERING_START  410
#define FFP_MSG_AUDIO_SEEK_RENDERING_START  411
#define FFP_MSG_CACHE_OPEN_SUCCESS          420

#define FFP_MSG_BUFFERING_START             500
#define FFP_MSG_BUFFERING_END               501
#define FFP_MSG_BUFFERING_UPDATE            502     /* arg1 = buffering head position in time, arg2 = minimum percent in time or bytes */
#define FFP_MSG_BUFFERING_BYTES_UPDATE      503     /* arg1 = cached data in bytes,            arg2 = high water mark */
#define FFP_MSG_BUFFERING_TIME_UPDATE       504     /* arg1 = cached duration in milliseconds, arg2 = high water mark */
#define FFP_MSG_SEEK_COMPLETE               600     /* arg1 = seek position,                   arg2 = error */
#define FFP_MSG_PLAYBACK_STATE_CHANGED      700
#define FFP_MSG_TIMED_TEXT                  800
#define FFP_MSG_ACCURATE_SEEK_COMPLETE      900     /* arg1 = current position*/
#define FFP_MSG_GET_IMG_STATE               1000    /* arg1 = timestamp, arg2 = result code, obj = file name*/
#define FFP_MSG_BUFFER_FAIL_WEAK_NETWORK    1010     /* in weak network, Read data fail*/
#define FFP_MSG_OPEN_URL_ERROR              1020    /* arg1 = error */
#define FFP_MSG_SWITCH_CLARITY_ERROR        1021    /* arg1 = error */
#define FFP_MSG_AAUDIO_DISCONNECT_ERROR_INNER 1022  /* aaudio disconnect error for inner */
#define FFP_MSG_AAUDIO_DISCONNECT_ERROR     1023
#define FFP_MSG_OPEN_SUBTITLE_ERROR         1024    /* arg1 = error */
#define FFP_MSG_RECONNECT_FAIL_AND_EXIT     1030    /* try to reconnect multiple time , fail and exit, should replay */
#define FFP_MSG_OPEN_CACHE_FILE_ERROR       1040    /* open cache file error and exit */
#define FFP_MSG_SEEK_ERROR                  1050    /* seek error */
#define FFP_MSG_DECODEC_OVERLOAD            1200
#define FFP_MSG_CONFIG_SURFACE_SUCCESS      1300
#define FFP_MSG_NOT_CONFIG_SURFACE          1301
#define FFP_MSG_URL_OVERSIZE                1302    /* url size larger than (URL_SIZE(1024) - 1)*/
#define FFP_MSG_URL_CACHE_ERROR             1303
#define FFP_MSG_AUTH_ERROR                  1400
#define FFP_SET_DECRYPTION_KEY_FAIL         1401

#define FFP_PLAY_NEXT_VIDEO_SUCC             5004    
#define FFP_SET_NEXT_VIDEO_SUCC              5002
#define FFP_SET_NEXT_VIDEO_FAIL              5003
#define FFP_MSG_MP4_CONFIG_PARSE_SUCC        5005


#define FFP_MSG_VIDEO_DECODER_OPEN          10001

#define FFP_REQ_START                       20001
#define FFP_REQ_PAUSE                       20002
#define FFP_REQ_SEEK                        20003

#define FFP_PROP_FLOAT_VIDEO_DECODE_FRAMES_PER_SECOND   10001
#define FFP_PROP_FLOAT_VIDEO_OUTPUT_FRAMES_PER_SECOND   10002
#define FFP_PROP_FLOAT_PLAYBACK_RATE                    10003
#define FFP_PROP_FLOAT_PLAYBACK_VOLUME                  10006
#define FFP_PROP_FLOAT_AVDELAY                          10004
#define FFP_PROP_FLOAT_AVDIFF                           10005
#define FFP_PROP_FLOAT_DROP_FRAME_RATE                  10007

#define FFP_PROP_INT64_SELECTED_VIDEO_STREAM            20001
#define FFP_PROP_INT64_SELECTED_AUDIO_STREAM            20002
#define FFP_PROP_INT64_SELECTED_TIMEDTEXT_STREAM        20011
#define FFP_PROP_INT64_VIDEO_DECODER                    20003
#define FFP_PROP_INT64_AUDIO_DECODER                    20004
#define     FFP_PROPV_DECODER_UNKNOWN                   0
#define     FFP_PROPV_DECODER_AVCODEC                   1
#define     FFP_PROPV_DECODER_MEDIACODEC                2
#define     FFP_PROPV_DECODER_VIDEOTOOLBOX              3
#define FFP_PROP_INT64_VIDEO_CACHED_DURATION            20005
#define FFP_PROP_INT64_AUDIO_CACHED_DURATION            20006
#define FFP_PROP_INT64_VIDEO_CACHED_BYTES               20007
#define FFP_PROP_INT64_AUDIO_CACHED_BYTES               20008
#define FFP_PROP_INT64_VIDEO_CACHED_PACKETS             20009
#define FFP_PROP_INT64_AUDIO_CACHED_PACKETS             20010

#define FFP_PROP_INT64_BIT_RATE                         20100

#define FFP_PROP_INT64_TCP_SPEED                        20200

#define FFP_PROP_INT64_ASYNC_STATISTIC_BUF_BACKWARDS    20201
#define FFP_PROP_INT64_ASYNC_STATISTIC_BUF_FORWARDS     20202
#define FFP_PROP_INT64_ASYNC_STATISTIC_BUF_CAPACITY     20203
#define FFP_PROP_INT64_TRAFFIC_STATISTIC_BYTE_COUNT     20204

#define FFP_PROP_INT64_LATEST_SEEK_LOAD_DURATION        20300

#define FFP_PROP_INT64_CACHE_STATISTIC_PHYSICAL_POS     20205

#define FFP_PROP_INT64_CACHE_STATISTIC_FILE_FORWARDS    20206

#define FFP_PROP_INT64_CACHE_STATISTIC_FILE_POS         20207

#define FFP_PROP_INT64_CACHE_STATISTIC_COUNT_BYTES      20208

#define FFP_PROP_INT64_LOGICAL_FILE_SIZE                20209
#define FFP_PROP_INT64_SHARE_CACHE_DATA                 20210
#define FFP_PROP_INT64_IMMEDIATE_RECONNECT              20211

#define HARMONY_VIDEO_CODEC_ENABLE                      1800000
#define MEDIACODEC_CODEC_ENABLE                         1717876
#define FFMPEG_CODEC_ENABLE                             1717877

#define MEDIACODEC_OUTPUT_BUFFER_OVERTIME               1717867   /* 超过5秒不出帧，可视为mediacodec解码出错，业务层应当Close后切到软解重试 */
#define MEDIACODEC_OUTPUT_BUFFER_ERROR                  1717868
#define MEDIACODEC_COLOR_FORMAT_ERROR                   1717869
#define MEDIACODEC_QUEUEINPUTBUFFER_ERROR               1717870
#define MEDIACODEC_CREATE_CODEC_ERROR                   1717871
#define MEDIACODEC_CONFIG_ERROR                         1717872
#define MEDIACODEC_START_ERROR                          1717873
#define MEDIACODEC_DELETE_ERROR                         1717874
#define MEDIACODEC_FLUSH_ERROR                          1717875

#define FFP_PANDORA_VIDEO_TAG                           31001
#define FFP_H5_VIDEO_TAG                                31002
#define FFP_PANDORA_NEXT_VIDEO_TAG                      31003


#endif
