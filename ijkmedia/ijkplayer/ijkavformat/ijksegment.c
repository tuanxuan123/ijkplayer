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


#include <sys/types.h>
#include <fcntl.h>
#include "libavformat/avformat.h"
#include "libavformat/url.h"
#include "libavutil/avstring.h"
#include "libavutil/opt.h"
#include "ijkplayer_delegate_def.h"




extern pixvideo_file_open_delegate g_pandora_file_open_del;
extern pixvideo_file_read_delegate g_pandora_file_read_del;
extern pixvideo_file_seek_delegate g_pandora_file_seek_del;
extern pixvideo_file_close_delegate g_pandora_file_close_del;

extern pixvideo_file_open_delegate g_h5_file_open_del;
extern pixvideo_file_read_delegate g_h5_file_read_del;
extern pixvideo_file_seek_delegate g_h5_file_seek_del;
extern pixvideo_file_close_delegate g_h5_file_close_del;

typedef struct FileContext {
    const AVClass   *class;
    int             fd;
    void            *handle;
    int             is_pixui;
    
} FileContext;


static const AVOption file_options[] = {
    { NULL }
};



static const AVClass ijksegment_context_class = {
    .class_name = "ijksegment",
    .item_name  = av_default_item_name,
    .option     = file_options,
    .version    = LIBAVUTIL_VERSION_INT,
};



static int file_open(URLContext *h, const char *filename, int flags)
{
    FileContext *c = h->priv_data;

    av_strstart(filename, "ijksegment:", &filename);

    c->is_pixui = av_strstart(filename, "pixui:", &filename) ? 1 : 0;
    c->handle = NULL;
    if(c->is_pixui) {
        if (g_h5_file_open_del)
        {
            c->handle = g_h5_file_open_del(filename, O_RDONLY);
        }
    } else {
        if (g_pandora_file_open_del)
        {
            c->handle = g_pandora_file_open_del(filename, O_RDONLY);
        }
    }

    if(c->handle)
        return 0;

    return -1;
}


static int file_read(URLContext *h, unsigned char *buf, int size)
{
    FileContext *c = h->priv_data;

    if(c->is_pixui) {
        if (g_h5_file_read_del)
        {
            return g_h5_file_read_del(c->handle, buf, size);
        }
    } else {
        if (g_pandora_file_read_del)
        {
            return g_pandora_file_read_del(c->handle, buf, size);        
        }
    }
    return -1;
}


static int64_t file_seek(URLContext *h, int64_t pos, int whence)
{
    FileContext *c = h->priv_data;
    
    if(c->is_pixui) {
        if (g_h5_file_seek_del)
        {
            return g_h5_file_seek_del(c->handle, pos, whence);
        }
    } else {
        if (g_pandora_file_seek_del)
        {
            return g_pandora_file_seek_del(c->handle, pos, whence);
        }
    }
    return -1;
}

static int file_close(URLContext *h)
{
    FileContext *c = h->priv_data;

    if(c->is_pixui) {
        if (g_h5_file_close_del)
        {
            return g_h5_file_close_del(c->handle);
        }
    } else {
        if (g_pandora_file_close_del)
        {
            return g_pandora_file_close_del(c->handle);
        }
    }
    return -1;
}







const URLProtocol ijkimp_ff_ijksegment_protocol = {
    .name                = "ijksegment",
    .url_open            = file_open,
    .url_read            = file_read,
    .url_seek            = file_seek,
    .url_close           = file_close,
    .priv_data_size      = sizeof(FileContext),
    .priv_data_class     = &ijksegment_context_class,
};






