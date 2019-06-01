/*
 * ffpipeline_native_android.h
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

#ifndef FFPLAY__FF_FFPIPELINE_NATIVE_ANDROID_H
#define FFPLAY__FF_FFPIPELINE_NATIVE_ANDROID_H

#include <stdbool.h>
#include "../ijkplayer_android_def.h"
#include "../../ff_ffpipeline.h"
#include "ijksdl/ijksdl_vout.h"

typedef struct FFPlayer       FFPlayer;
typedef struct IJKFF_Pipeline IJKFF_Pipeline;

IJKFF_Pipeline *ffpipeline_create_from_android(FFPlayer *ffp);

void    ffpipeline_set_vout(IJKFF_Pipeline* pipeline, SDL_Vout *vout);

void    ffpipeline_set_mediacodec_select_callback(IJKFF_Pipeline* pipeline, bool (*callback)(void *opaque, ijkmp_mediacodecinfo_context *mcc), void *opaque);

void    ffpipeline_set_volume(IJKFF_Pipeline* pipeline, float left, float right);

#endif
