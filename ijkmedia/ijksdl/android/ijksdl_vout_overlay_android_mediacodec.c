/*****************************************************************************
 * ijksdl_vout_overlay_android_mediacodec.c
 *****************************************************************************
 *
 * Copyright (c) 2014 Bilibili
 * copyright (c) 2014 Zhang Rui <bbcallen@gmail.com>
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

#include "ijksdl_vout_overlay_android_mediacodec.h"
#include "ijksdl/ijksdl_stdinc.h"
#include "ijksdl/ijksdl_mutex.h"
#include "ijksdl/ijksdl_vout_internal.h"
#include "ijksdl/ijksdl_video.h"

typedef struct SDL_VoutOverlay_Opaque {
    SDL_mutex                  *mutex;
    SDL_Vout                   *vout;
} SDL_VoutOverlay_Opaque;

static void overlay_release_output_buffer(SDL_VoutOverlay *overlay)
{
    if (overlay->output_buffer_index >= 0) {
        SDL_VoutOverlay_Opaque *opaque = overlay->opaque;

        overlay->render = 0;
        SDL_Vout *vout = opaque->vout;
        if (vout && vout->display_overlay) {
            vout->display_overlay(vout, overlay);
        }
    }
}

static int overlay_lock(SDL_VoutOverlay *overlay)
{
    SDL_VoutOverlay_Opaque *opaque = overlay->opaque;
    return SDL_LockMutex(opaque->mutex);
}

static int overlay_unlock(SDL_VoutOverlay *overlay)
{
    SDL_VoutOverlay_Opaque *opaque = overlay->opaque;
    return SDL_UnlockMutex(opaque->mutex);
}

static void overlay_unref(SDL_VoutOverlay *overlay)
{
    if (!overlay) {
        return;
    }

    if (overlay->output_buffer_index >= 0) {
        overlay_release_output_buffer(overlay);
    }
}

static void overlay_free_l(SDL_VoutOverlay *overlay)
{
    if (!overlay || !overlay->opaque) {
        return;
    }
        
    SDL_VoutOverlay_Opaque *opaque = overlay->opaque;

    overlay_unref(overlay);

    if (opaque->mutex)
        SDL_DestroyMutex(opaque->mutex);

    SDL_VoutOverlay_FreeInternal(overlay);
}


static int func_fill_frame(SDL_VoutOverlay *overlay, const AVFrame *frame)
{
    if (overlay->output_buffer_index >= 0) {
        overlay_release_output_buffer(overlay);
    }

    overlay->output_buffer_index = (int)frame->opaque;
    overlay->render     = 1;
    overlay->format     = SDL_FCC__AMC;
    overlay->planes     = 1;
    overlay->is_private = 1;
    overlay->w = frame->width;
    overlay->h = frame->height;

    return 0;
}

SDL_VoutOverlay *SDL_VoutAMediaCodec_CreateOverlay(int width, int height, SDL_Vout *vout)
{
    SDLTRACE("SDL_VoutAMediaCodec_CreateOverlay(w=%d, h=%d, fmt=_AMC vout=%p)\n", width, height, vout);
    SDL_VoutOverlay *overlay = SDL_VoutOverlay_CreateInternal(sizeof(SDL_VoutOverlay_Opaque));
    if (!overlay) {
        ALOGE("overlay allocation failed");
        return NULL;
    }

    SDL_VoutOverlay_Opaque *opaque = overlay->opaque;
    opaque->mutex         = SDL_CreateMutex();
    opaque->vout          = vout;
    overlay->output_buffer_index = -1;
    overlay->format       = SDL_FCC__AMC;
    overlay->pitches      = NULL;
    overlay->pixels       = NULL;
    overlay->w            = width;
    overlay->h            = height;
    overlay->is_private   = 1;

    overlay->free_l       = overlay_free_l;
    overlay->lock         = overlay_lock;
    overlay->unlock       = overlay_unlock;
    overlay->unref        = overlay_unref;
    overlay->func_fill_frame = func_fill_frame;

    if (!opaque->mutex) {
        ALOGE("SDL_CreateMutex failed");
        goto fail;
    }

    return overlay;

fail:
    overlay_free_l(overlay);
    return NULL;
}

