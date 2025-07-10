/*****************************************************************************
 * ijksdl_vout_overlay_videotoolbox.m
 *****************************************************************************
 *
 * copyright (c) 2014 ZhouQuan <zhouqicy@gmail.com>
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

#include "ijksdl_vout_overlay_videotoolbox.h"
#include "ijksdl_stdinc.h"
#include "ijksdl_mutex.h"
#include "ijksdl_vout_internal.h"
#include "ijksdl_video.h"


struct SDL_VoutOverlay_Opaque {
    SDL_mutex *mutex;
    CVPixelBufferRef pixel_buffer;
    CVMetalTextureRef y_texture;
    CVMetalTextureRef uv_texture;
    Uint16 pitches[AV_NUM_DATA_POINTERS];
    Uint8 *pixels[AV_NUM_DATA_POINTERS];
};


static CVMetalTextureCacheRef s_metal_texture_cache = nil;
static id<MTLDevice> s_metal_device = nil;           

void SDL_SetMetalDevice(void* device)
{
    if (device) {
        s_metal_device = (__bridge id<MTLDevice>)device;
    }
}

static void create_metal_texturecache()
{
    if (s_metal_texture_cache != nil) {
        return;
    }

    if (s_metal_device == nil) {
        NSLog(@"create metal texture cache fail, device is null\n");
        return;
    }

    CVReturn result = CVMetalTextureCacheCreate(kCFAllocatorDefault, nil, s_metal_device, nil, &s_metal_texture_cache);
    if (result != kCVReturnSuccess) {
        s_metal_texture_cache = nil;
        NSLog(@"create metal texture cache fail, resule:%d \n", result);
    }
}


static void func_free_l(SDL_VoutOverlay *overlay)
{
    if (!overlay)
        return;
    SDL_VoutOverlay_Opaque *opaque = overlay->opaque;
    if (!opaque)
        return;
    overlay->unref(overlay);
    if (opaque->mutex)
        SDL_DestroyMutex(opaque->mutex);

    SDL_VoutOverlay_FreeInternal(overlay);
}

static int func_lock(SDL_VoutOverlay *overlay)
{
    SDL_VoutOverlay_Opaque *opaque = overlay->opaque;
    return SDL_LockMutex(opaque->mutex);
}

static int func_unlock(SDL_VoutOverlay *overlay)
{
    SDL_VoutOverlay_Opaque *opaque = overlay->opaque;
    return SDL_UnlockMutex(opaque->mutex);
}

static void func_unref(SDL_VoutOverlay *overlay)
{
    if (!overlay) {
        return;
    }
    SDL_VoutOverlay_Opaque *opaque = overlay->opaque;
    if (!opaque) {
        return;
    }

    if (opaque->pixel_buffer != NULL) {
        CVBufferRelease(opaque->pixel_buffer);
    }

    if (opaque->y_texture != nil) {
        CFRelease(opaque->y_texture);
        CFRelease(opaque->uv_texture);
        opaque->y_texture = nil;
        opaque->uv_texture = nil;
    }

    opaque->pixel_buffer = NULL;
    overlay->pixels[0] = NULL;
    overlay->pixels[1] = NULL;

    return;
}

static int func_fill_frame(SDL_VoutOverlay *overlay, const AVFrame *frame)
{
    CVBufferRef pixel_buffer = CVBufferRetain(frame->opaque);
    SDL_VoutOverlay_Opaque *opaque = overlay->opaque;
    if (opaque->pixel_buffer != NULL) {
        CVBufferRelease(opaque->pixel_buffer);
    }

    if (opaque->y_texture != nil) {
        CFRelease(opaque->y_texture);
        CFRelease(opaque->uv_texture);
        opaque->y_texture = nil;
        opaque->uv_texture = nil;
    }
    
    opaque->pixel_buffer = pixel_buffer;
    overlay->planes = 2;
    overlay->pitches[0] = CVPixelBufferGetWidthOfPlane(pixel_buffer, 0);
    overlay->pitches[1] = CVPixelBufferGetHeightOfPlane(pixel_buffer, 0);
    overlay->is_private = 1;
    overlay->w = (int)frame->width;
    overlay->h = (int)frame->height;

    if (frame->format == IJK_AV_PIX_FMT__VIDEO_TOOLBOX) {
        //get yuv data
        if (CVPixelBufferLockBaseAddress(pixel_buffer, 0) != kCVReturnSuccess) {
            overlay->pixels[0]  = NULL;
            overlay->pixels[1]  = NULL;
            overlay->pitches[0] = 0;
            overlay->pitches[1] = 0;
            overlay->w = 0;
            overlay->h = 0;
            CVBufferRelease(pixel_buffer);
            opaque->pixel_buffer = NULL;
            return -1;
        }
        
        overlay->pixels[0]  = CVPixelBufferGetBaseAddressOfPlane(pixel_buffer, 0);  //y data
        overlay->pixels[1]  = CVPixelBufferGetBaseAddressOfPlane(pixel_buffer, 1);  //uv data
        CVPixelBufferUnlockBaseAddress(pixel_buffer, 0);
        overlay->format = SDL_FCC__VTB;
    } else {
        //get yuv metal texture
        create_metal_texturecache();

        overlay->format = SDL_FCC_T_VTB;

        CVMetalTextureCacheCreateTextureFromImage(
            kCFAllocatorDefault,
            s_metal_texture_cache,
            pixel_buffer,
            nil,
            MTLPixelFormatR8Unorm,
            overlay->pitches[0],
            overlay->pitches[1],
            0,
            &opaque->y_texture
        );

        CVMetalTextureCacheCreateTextureFromImage(
            kCFAllocatorDefault,
            s_metal_texture_cache,
            pixel_buffer,
            nil,
            MTLPixelFormatRG8Unorm,
            CVPixelBufferGetWidthOfPlane(pixel_buffer, 1),
            CVPixelBufferGetHeightOfPlane(pixel_buffer, 1),
            1,
            &opaque->uv_texture
        );

        overlay->pixels[0] = (__bridge uint8_t*)opaque->y_texture;
        overlay->pixels[1] = (__bridge uint8_t*)opaque->uv_texture;
        overlay->pixels[2] = (__bridge uint8_t*)opaque->pixel_buffer;
    }

    return 0;
}

static SDL_Class g_vout_overlay_videotoolbox_class = {
    .name = "VideoToolboxVoutOverlay",
};

static bool check_object(SDL_VoutOverlay* object, const char *func_name)
{
    if (!object || !object->opaque || !object->opaque_class) {
        ALOGE("%s: invalid pipeline\n", func_name);
        return false;
    }

    if (object->opaque_class != &g_vout_overlay_videotoolbox_class) {
        ALOGE("%s.%s: unsupported method\n", object->opaque_class->name, func_name);
        return false;
    }

    return true;
}

CVPixelBufferRef SDL_VoutOverlayVideoToolBox_GetCVPixelBufferRef(SDL_VoutOverlay *overlay)
{
    if (!check_object(overlay, __func__))
        return NULL;

    SDL_VoutOverlay_Opaque *opaque = overlay->opaque;
    return opaque->pixel_buffer;
}

SDL_VoutOverlay *SDL_VoutVideoToolBox_CreateOverlay(int width, int height, SDL_Vout *display)
{
    SDLTRACE("SDL_VoutVideoToolBox_CreateOverlay(w=%d, h=%d, fmt=_VTB, dp=%p)\n",
             width, height, display);
    SDL_VoutOverlay *overlay = SDL_VoutOverlay_CreateInternal(sizeof(SDL_VoutOverlay_Opaque));
    if (!overlay) {
        ALOGE("overlay allocation failed");
        return NULL;
    }
    memset(overlay->opaque, 0, sizeof(SDL_VoutOverlay_Opaque));
    SDL_VoutOverlay_Opaque *opaque = overlay->opaque;
    overlay->opaque_class = &g_vout_overlay_videotoolbox_class;
    overlay->format     = SDL_FCC__VTB;
    overlay->w          = width;
    overlay->h          = height;
    overlay->pitches    = opaque->pitches;
    overlay->pixels     = opaque->pixels;
    overlay->is_private = 1;

    overlay->free_l             = func_free_l;
    overlay->lock               = func_lock;
    overlay->unlock             = func_unlock;
    overlay->unref              = func_unref;
    overlay->func_fill_frame    = func_fill_frame;

    opaque->mutex = SDL_CreateMutex();
    return overlay;
}
