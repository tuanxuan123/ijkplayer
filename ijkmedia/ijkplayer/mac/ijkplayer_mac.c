
#include "ijkplayer_mac.h"
#include "ff_ffmsg.h"
#include "ijkplayer_internal.h"
#include "ijksdl_vout_mac_null.h"
#include "ffpipeline_mac.h"


IjkMediaPlayer *ijkmp_mac_create(int (*msg_loop)(void*))
{
    IjkMediaPlayer *mp = ijkmp_create(msg_loop);
    if (!mp)
        goto fail;


    mp->ffplayer->vout = SDL_VoutMac_CreateForNull();
    if (!mp->ffplayer->vout)
        goto fail;


    mp->ffplayer->pipeline = ffpipeline_create_from_mac(mp->ffplayer);
    if (!mp->ffplayer->pipeline)
        goto fail;

    return mp;

fail:
    ijkmp_destroy(mp);
    return NULL;
}
