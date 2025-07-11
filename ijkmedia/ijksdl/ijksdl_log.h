/*****************************************************************************
 * ijksdl_log.h
 *****************************************************************************
 *
 * Copyright (c) 2015 Bilibili
 * copyright (c) 2015 Zhang Rui <bbcallen@gmail.com>
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

#ifndef IJKSDL__IJKSDL_LOG_H
#define IJKSDL__IJKSDL_LOG_H

#include <stdio.h>

#define LOG_BUFFER_SIZE         1024

#ifdef _WIN32
#define EXPORT_API  __declspec(dllexport)
#else
#define EXPORT_API  __attribute__ ((visibility ("default")))
#endif

EXPORT_API void pixvideo_set_log_level(int level);
EXPORT_API int 	pixvideo_get_log_level(); 

#ifdef __ANDROID__

#include <android/log.h>
#include "ijksdl_extra_log.h"

#define IJK_LOG_UNKNOWN     ANDROID_LOG_UNKNOWN
#define IJK_LOG_DEFAULT     ANDROID_LOG_DEFAULT

#define IJK_LOG_VERBOSE     ANDROID_LOG_VERBOSE
#define IJK_LOG_DEBUG       ANDROID_LOG_DEBUG
#define IJK_LOG_INFO        ANDROID_LOG_INFO
#define IJK_LOG_WARN        ANDROID_LOG_WARN
#define IJK_LOG_ERROR       ANDROID_LOG_ERROR
#define IJK_LOG_FATAL       ANDROID_LOG_FATAL
#define IJK_LOG_SILENT      ANDROID_LOG_SILENT

#ifdef EXTRA_LOG_PRINT
#define VLOG(level, TAG, ...)    if(pixvideo_get_log_level() <= (int)level) {ffp_log_extra_vprint(level, TAG, __VA_ARGS__);}
#define ALOG(level, TAG, ...)    if(pixvideo_get_log_level() <= (int)level) {ffp_log_extra_print(level, TAG, __VA_ARGS__);}
#else
#define VLOG(level, TAG, ...)    if(pixvideo_get_log_level() <= (int)level) {(void)__android_log_vprint(level, TAG, __VA_ARGS__);}
#define ALOG(level, TAG, ...)    if(pixvideo_get_log_level() <= (int)level) {(void)__android_log_print(level, TAG, __VA_ARGS__);}
#endif

#else

#define IJK_LOG_UNKNOWN     0
#define IJK_LOG_DEFAULT     1

#define IJK_LOG_VERBOSE     2
#define IJK_LOG_DEBUG       3
#define IJK_LOG_INFO        4
#define IJK_LOG_WARN        5
#define IJK_LOG_ERROR       6
#define IJK_LOG_FATAL       7
#define IJK_LOG_SILENT      8

#ifdef  _WIN32

EXPORT_API void pixvideo_ffp_log_windows_print(int level, const char* tag, const char* msg);

#define VLOG(level, TAG, ...)                          \
	if (pixvideo_get_log_level() <= level)              \
	{                                                  \
		(void) vprintf(__VA_ARGS__);                   \
		char log_buffer[LOG_BUFFER_SIZE] = {0};        \
		vsnprintf(log_buffer, LOG_BUFFER_SIZE, __VA_ARGS__);      \
		pixvideo_ffp_log_windows_print(level, TAG, log_buffer); \
	}
#define ALOG(level, TAG, ...)                          \
	if (pixvideo_get_log_level() <= level)              \
	{                                                  \
		(void) printf(__VA_ARGS__);                    \
		char log_buffer[LOG_BUFFER_SIZE] = {0};        \
		snprintf(log_buffer, LOG_BUFFER_SIZE, __VA_ARGS__);              \
		pixvideo_ffp_log_windows_print(level, TAG, log_buffer); \
	}

#elif defined(OHOS)
#include "hilog/log.h"
#define VLOG(level, TAG, ...)                          \
	if (pixvideo_get_log_level() <= level)              \
	{                                                  \
		(void) vprintf(__VA_ARGS__);                   \
		char log_buffer[LOG_BUFFER_SIZE] = {0};        \
		vsnprintf(log_buffer, LOG_BUFFER_SIZE, __VA_ARGS__);      \
		OH_LOG_Print(LOG_APP, level, LOG_DOMAIN, TAG, "%{public}s", log_buffer); \
	}
#define ALOG(level, TAG, ...)                          \
	if (pixvideo_get_log_level() <= level)              \
	{                                                  \
		(void) printf(__VA_ARGS__);                    \
		char log_buffer[LOG_BUFFER_SIZE] = {0};        \
		snprintf(log_buffer, LOG_BUFFER_SIZE, __VA_ARGS__);              \
		OH_LOG_Print(LOG_APP, level, LOG_DOMAIN, TAG, " %{public}s", log_buffer); \
	}
#else

#define VLOG(level, TAG, ...)    if(pixvideo_get_log_level() <= level) {(void)vprintf(__VA_ARGS__);}
#define ALOG(level, TAG, ...)    if(pixvideo_get_log_level() <= level) {(void)printf(__VA_ARGS__);}

#endif //  _WIN32




#endif

#define IJK_LOG_TAG "PANDORAMEDIA"

#define VLOGV(...)  VLOG(IJK_LOG_VERBOSE,   IJK_LOG_TAG, __VA_ARGS__)
#define VLOGD(...)  VLOG(IJK_LOG_DEBUG,     IJK_LOG_TAG, __VA_ARGS__)
#define VLOGI(...)  VLOG(IJK_LOG_INFO,      IJK_LOG_TAG, __VA_ARGS__)
#define VLOGW(...)  VLOG(IJK_LOG_WARN,      IJK_LOG_TAG, __VA_ARGS__)
#define VLOGE(...)  VLOG(IJK_LOG_ERROR,     IJK_LOG_TAG, __VA_ARGS__)

#define ALOGV(...)  ALOG(IJK_LOG_VERBOSE,   IJK_LOG_TAG, __VA_ARGS__)
#define ALOGD(...)  ALOG(IJK_LOG_DEBUG,     IJK_LOG_TAG, __VA_ARGS__)
#define ALOGI(...)  ALOG(IJK_LOG_INFO,      IJK_LOG_TAG, __VA_ARGS__)
#define ALOGW(...)  ALOG(IJK_LOG_WARN,      IJK_LOG_TAG, __VA_ARGS__)
#define ALOGE(...)  ALOG(IJK_LOG_ERROR,     IJK_LOG_TAG, __VA_ARGS__)
#define LOG_ALWAYS_FATAL(...)   do { ALOGE(__VA_ARGS__); exit(1); } while (0)




#endif
