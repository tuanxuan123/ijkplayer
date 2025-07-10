/*
 * Copyright (c) 2015 Bilibili
 * Copyright (c) 2015 Zhang Rui <bbcallen@gmail.com>
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

#ifndef FFPLAY__CONFIG_H
#define FFPLAY__CONFIG_H

#if !_WIN32
#include "libffmpeg/config.h"
#endif // !_WIN32



// FIXME: merge filter related code and enable it
// remove these lines to enable avfilter
#ifdef CONFIG_AVFILTER
#undef CONFIG_AVFILTER
#endif
#define CONFIG_AVFILTER 0

#ifdef FFP_MERGE
#undef FFP_MERGE
#endif

#ifdef FFP_SUB
#undef FFP_SUB
#endif

#ifndef FFMPEG_LOG_TAG
#define FFMPEG_LOG_TAG "IJKFFMPEG"
#endif

#if _WIN32 || TARGET_OS_MAC
#define MAX_VIDEO_COUNT 			32
#else
#define MAX_VIDEO_COUNT             16
#endif

#define MAX_MEDIACODEC_COUNT 		3
#define MAX_PATH_LEN                1024
#define MAX_SWITCH_PLAY_COUNT       360
#define SWITCH_VOD_MS               300

#define CACHE_INDEX_TYPE            ".vindex"
#define CACHE_DATA_TYPE             ".vdata"
#define CACHE_PROTOCAL              "ijkio:cache:ffio:"
#define WITHOUT_IJKIO_CACHE_PROTOCAL  "cache:ffio:" /* 不进行ijkio的缓存协议头 */



#endif//FFPLAY__CONFIG_H
