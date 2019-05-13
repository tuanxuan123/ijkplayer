
#include "ijkplayer_internal.h"
#include "h5_player.h"
#include "ff_ffmsg.h"


#ifdef _WIN32

#include "windows/ijkplayer_windows.h"

#elif TARGET_OS_MAC

#include "ijkplayer_mac.h"
#elif TARGET_OS_IPHONE
#include "ijkplayer_ios.h"
#elif __ANDROID__
#include "android/ijkplayer_android.h"
 	
#endif




static int					s_video_index = 0;
static h5_msg_callback		s_msg_callback = NULL;
static h5_video_callback 	s_player_callback = NULL;
static IjkMediaPlayer 		*s_media_player = NULL;
static MessageQueue 		s_player_msg_queue;
static void deal_yuv420p_data(SDL_VoutOverlay* overlay);
static void deal_yuv420sp_data(SDL_VoutOverlay* overlay);


static void frame_update_callback(SDL_VoutOverlay* overlay)
{
	if (!s_player_callback)
		return;

	switch (overlay->format)
	{
		case SDL_FCC_I420:
		case SDL_FCC_YV12:
			deal_yuv420p_data(overlay);
			break;
		case SDL_FCC_NV12:
		case SDL_FCC__VTB:		
			deal_yuv420sp_data(overlay);
			break;
		case SDL_FCC_RV16:      ALOGE("not support SDL_FCC_RV16\n"); break;
		case SDL_FCC_RV24:      ALOGE("not support SDL_FCC_RV24\n"); break;
		case SDL_FCC_RV32:      ALOGE("not support SDL_FCC_RV32\n"); break;
		default:				ALOGE("can not match format"); break;
	}
	
}



static bool transform_yuv2rgba(int w, int h, const uint8_t* yuv_data[], int yuv_linesize[], uint8_t* rgba_data[], int rgba_linesize[], enum AVPixelFormat yuv_format)
{
    //AV_PIX_FMT_NV12 AV_PIX_FMT_YUV420P
	struct SwsContext* context = sws_getContext(w, h, yuv_format, w, h, AV_PIX_FMT_RGBA, SWS_FAST_BILINEAR, NULL, NULL, NULL);

	if (context)
	{
		sws_scale(context, yuv_data, yuv_linesize, 0, h, rgba_data, rgba_linesize);
		sws_freeContext(context);
		context = NULL;
		return true;
	}
	else
	{
		ALOGE("sws_getContext create failed!!!");
		return false;
	}

}

static void deal_yuv420p_data(SDL_VoutOverlay* overlay)
{
	int w = overlay->w;
	int h = overlay->h;

	uint8_t* pixels[3];

	pixels[0] = overlay->pixels[0];
	if (overlay->format == SDL_FCC_YV12)
	{
		pixels[1] = overlay->pixels[2];
		pixels[2] = overlay->pixels[1];
	}
	else
	{
		pixels[1] = overlay->pixels[1];
		pixels[2] = overlay->pixels[2];
	}

	uint8_t *yuv_data[4];
	int yuv_linesize[4];

	uint8_t *rgba_data[4];
	int rgba_linesize[4];

	av_image_alloc(yuv_data, yuv_linesize, w, h, AV_PIX_FMT_YUV420P, 1);
	av_image_alloc(rgba_data, rgba_linesize, w, h, AV_PIX_FMT_RGBA, 1);


	if (w == overlay->pitches[0])
	{
		memcpy(yuv_data[0], pixels[0], w*h);
		memcpy(yuv_data[1], pixels[1], w*h / 4);
		memcpy(yuv_data[2], pixels[2], w*h / 4);
	}
	else
	{

		for (int row = 0; row < h; ++row)
		{
			memcpy(yuv_data[0] + row * w, pixels[0] + row * overlay->pitches[0], w);
		}

		for (int i = 1; i < 3; ++i)
		{
			for (int row = 0; row < h / 2; ++row)
			{
				memcpy(yuv_data[i] + row * w / 2, pixels[i] + row * overlay->pitches[0] / 2, w / 2);
			}
		}

	}
    
	if(transform_yuv2rgba(w, h, (const uint8_t**)&yuv_data, yuv_linesize, rgba_data, rgba_linesize, AV_PIX_FMT_YUV420P))
		s_player_callback(s_video_index, w, h, rgba_data[0]);
	

	av_freep(yuv_data);
	av_freep(rgba_data);
}


static void deal_yuv420sp_data(SDL_VoutOverlay* overlay)
{
	int w = overlay->w;
	int h = overlay->h;

	uint8_t* pixels[2];
	pixels[0] = overlay->pixels[0];
	pixels[1] = overlay->pixels[1];

	uint8_t *yuv_data[4];
	int yuv_linesize[4];

	uint8_t *rgba_data[4];
	int rgba_linesize[4];

	av_image_alloc(yuv_data, yuv_linesize, w, h, AV_PIX_FMT_NV12, 1);
	av_image_alloc(rgba_data, rgba_linesize, w, h, AV_PIX_FMT_RGBA, 1);

	//maybe need to do align
	memcpy(yuv_data[0], pixels[0], w*h);
    memcpy(yuv_data[1], pixels[1], w*h/2);
    

	if (transform_yuv2rgba(w, h, (const uint8_t**)&yuv_data, yuv_linesize, rgba_data, rgba_linesize, AV_PIX_FMT_NV12))
		s_player_callback(s_video_index, w, h, rgba_data[0]);


	av_freep(yuv_data);
	av_freep(rgba_data);

}



static int ijkff_inject_callback(void *opaque, int message, void *data, size_t data_size)
{

    return 0;
}

static int media_player_msg_loop(void* arg)
{
	IjkMediaPlayer *mp = (IjkMediaPlayer*)arg;

	int retval = 0;
    while(retval >= 0)
    {
        AVMessage msg;
        retval = ijkmp_get_msg(mp, &msg, 1);
        
        if(retval >= 0)
        {
            msg_queue_put(&s_player_msg_queue, &msg);
        }
                
    }
    
    ijkmp_dec_ref_p(&mp);

	return 0;
}


#ifdef _WIN32
void create_window_player()
{
	s_media_player = ijkmp_windows_create(media_player_msg_loop);
}


#elif TARGET_OS_MAC
void create_mac_player()
{
	s_media_player = ijkmp_mac_create(media_player_msg_loop);

}

#elif TARGET_OS_IPHONE
void create_ios_player()
{
    s_media_player = ijkmp_ios_create(media_player_msg_loop);
    av_log_set_level(AV_LOG_ERROR);
    
    ijkmp_set_option_int(s_media_player, IJKMP_OPT_CATEGORY_PLAYER, "videotoolbox", 1);  
}

#elif __ANDROID__
void create_android_player()
{
	s_media_player = ijkmp_android_create(media_player_msg_loop);
	ijkmp_set_option_int(s_media_player, IJKMP_OPT_CATEGORY_PLAYER, "mediacodec", 0);
	ijkmp_set_option_int(s_media_player, IJKMP_OPT_CATEGORY_PLAYER, "opensles", 1);  
}

#endif


void h5_video_init(h5_video_callback video_func, h5_msg_callback msg_func)
{
	s_player_callback = video_func;
	s_msg_callback = msg_func;

	if (s_media_player)
	{
		h5_video_destroy();
	}

	ijkmp_global_init();
    ijkmp_global_set_inject_callback(ijkff_inject_callback);
    msg_queue_init(&s_player_msg_queue);


#ifdef _WIN32
 	create_window_player();
#elif TARGET_OS_MAC
 	create_mac_player();
#elif TARGET_OS_IPHONE
 	create_ios_player();

#elif __ANDROID__
 	create_android_player();
#endif


    
    if (!s_media_player)
	{
		ALOGE("h5_video_init s_media_player is null!");
		return;
	}

	ijkmp_set_option_int(s_media_player, IJKMP_OPT_CATEGORY_PLAYER, "start-on-prepared", 1);   
    ijkmp_set_option(s_media_player, IJKMP_OPT_CATEGORY_PLAYER, "overlay-format", "fcc-_es2");

    ijkmp_set_option_int(s_media_player, IJKMP_OPT_CATEGORY_PLAYER, "max-fps", 30);
    ijkmp_set_option_int(s_media_player, IJKMP_OPT_CATEGORY_PLAYER, "framedrop", 1); 
    ijkmp_set_option_int(s_media_player, IJKMP_OPT_CATEGORY_PLAYER, "video-pictq-size", 3);

    ijkmp_set_option_int(s_media_player, IJKMP_OPT_CATEGORY_FORMAT, "reconnect", 1);
    ijkmp_set_option_int(s_media_player, IJKMP_OPT_CATEGORY_FORMAT, "timeout", 30 * 1000 * 1000);
    ijkmp_set_option(s_media_player, IJKMP_OPT_CATEGORY_FORMAT, "user-agent", "h5player");

    ijkmp_set_frame_callback(s_media_player, frame_update_callback);

}



void  h5_video_play(const char* url, bool is_loop, int index)
{
	if (!s_media_player)
	{
		ALOGE("h5_video_play s_media_player is null!");
		return;
	}


	s_video_index = index;
	ijkmp_set_data_source(s_media_player, url);
	ijkmp_set_option(s_media_player, IJKMP_OPT_CATEGORY_FORMAT, "safe", "0"); 
	ijkmp_prepare_async(s_media_player);    
    msg_queue_start(&s_player_msg_queue);

	if(is_loop)
		ijkmp_set_loop(s_media_player, 100000);
    
	ijkmp_start(s_media_player);

}

void h5_video_pause()
{
	if (!s_media_player)
        return;

    ijkmp_pause(s_media_player);
}


void h5_video_resume()
{
	if (!s_media_player)
        return;

    if(s_media_player->mp_state != MP_STATE_PAUSED)
    	return;

    ijkmp_start(s_media_player);
}




void h5_video_seek(int sec)
{
	if (!s_media_player)
        return;

    long msec = (long)sec * 1000;

    ijkmp_seek_to(s_media_player, msec);
}


long h5_video_get_duration()
{
	if (!s_media_player)
		return 0;

	return ijkmp_get_duration(s_media_player);
}

long h5_video_get_position()
{
	if (!s_media_player)
		return 0;

	return ijkmp_get_current_position(s_media_player);
}

void h5_video_update()
{
	if (!s_media_player)
		return;

#if !REFRESH_VIDEO_BY_IJKPLAYER
	ijkmp_update(s_media_player);
#endif // REFRESH_VIDEO_BY_IJKPLAYER

	AVMessage msg;

	while (1)
	{
		int retval = msg_queue_get(&s_player_msg_queue, &msg, 0);

		if (retval <= 0)
			break;

		if (s_msg_callback)
			s_msg_callback(s_video_index, msg.what, msg.arg1, msg.arg2, msg.obj);

		msg_free_res(&msg);
	}
}


void h5_video_stop()
{
	if (!s_media_player)
        return;

    msg_queue_abort(&s_player_msg_queue);
    ijkmp_stop(s_media_player);
}



void h5_video_destroy()
{
	if (!s_media_player)
        return;

	msg_queue_abort(&s_player_msg_queue);

    ijkmp_stop(s_media_player);
    ijkmp_shutdown(s_media_player);   
    ijkmp_dec_ref_p(&s_media_player); 

    s_media_player = NULL;


    ijkmp_global_uninit();
    msg_queue_destroy(&s_player_msg_queue);

}





void h5_video_set_option(const char* name, int value)
{
	if (!s_media_player)
        return;

    ijkmp_set_option_int(s_media_player, IJKMP_OPT_CATEGORY_FORMAT, name, value);
}

