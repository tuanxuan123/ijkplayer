

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <errno.h>

#include "ijkplayer_internal.h"
#include "ff_ffmsg.h"
#include "h5_player.h"
#define  _CRT_SECURE_NO_WARNINGS
#ifdef _WIN32
#include "windows/ijkplayer_windows.h"
#elif TARGET_OS_MAC

#include "ijkplayer_mac.h"
#elif TARGET_OS_IPHONE
#include "ijkplayer_ios.h"
#elif __ANDROID__
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <dirent.h>
#include "android/ijkplayer_android.h"
#endif



static bool					s_global_init = false;
static int					s_video_index = 0;
static h5_msg_callback		s_msg_callback = NULL;
static h5_video_callback 	s_player_callback = NULL;
static IjkMediaPlayer 		*s_media_player = NULL;
static enum AVPixelFormat   s_pix_format = AV_PIX_FMT_RGB24;
static MessageQueue 		s_player_msg_queue;
static void deal_yuv420p_data(SDL_VoutOverlay* overlay);
static void deal_yuv420sp_data(SDL_VoutOverlay* overlay);
//add
static char*				cache_path = NULL;

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
		default:				ALOGE("can not match format: %d\n", overlay->format); break;
	}
	
}



static bool transform_yuv2rgb(int w, int h, const uint8_t* yuv_data[], int yuv_linesize[], uint8_t* rgba_data[], int rgba_linesize[], enum AVPixelFormat yuv_format)
{
    //AV_PIX_FMT_NV12 AV_PIX_FMT_YUV420P
	struct SwsContext* context = sws_getContext(w, h, yuv_format, w, h, s_pix_format, SWS_FAST_BILINEAR, NULL, NULL, NULL);

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

static void send_rbg_data(SDL_VoutOverlay* overlay, uint8_t *dst_data)
{
    if(!s_player_callback)
        return;

    int w = overlay->w;
    int h = overlay->h;

    if (w == overlay->pitches[0] && h == overlay->pitches[1])
    {
        s_player_callback(s_video_index, w, h, dst_data);
    }
    else
    {
        int pix_bit = s_pix_format == AV_PIX_FMT_RGB24? 3 : 4;
        int width_bytes = w * pix_bit * sizeof(uint8_t);
        int pitch_bytes = overlay->pitches[0] * pix_bit * sizeof(uint8_t);
        uint8_t* temp_data = malloc(width_bytes * h);

        for(int i = 0; i < h; i++)
        {
            memcpy(temp_data + i * width_bytes, dst_data + i * pitch_bytes, width_bytes);
        }
        s_player_callback(s_video_index, w, h, temp_data);

        free(temp_data);
    }
}

static void deal_yuv420p_data(SDL_VoutOverlay* overlay)
{
    if(!overlay)
        return;

	int w = overlay->pitches[0];   //line_width
	int h = overlay->pitches[1];   //line_height


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

	uint8_t *dst_data[4];
	int dst_linesize[4];

	av_image_alloc(yuv_data, yuv_linesize, w, h, AV_PIX_FMT_YUV420P, 1);
	av_image_alloc(dst_data, dst_linesize, w, h, s_pix_format, 1);


    memcpy(yuv_data[0], pixels[0], w*h);
    memcpy(yuv_data[1], pixels[1], w*h / 4);
    memcpy(yuv_data[2], pixels[2], w*h / 4);

    /*if (w == overlay->pitches[0])
    {
        memcpy(yuv_data[0], pixels[0], w*h);
        memcpy(yuv_data[1], pixels[1], w*h / 4);
        memcpy(yuv_data[2], pixels[2], w*h / 4);
    }
    else
    {
        for (int plane = 0; plane < 3; ++plane)
        {
            for (int row = 0; row < heights[plane]; ++row)
            {
                memcpy(yuv_data[plane] + row * widths[plane], pixels[plane] + row * linesizes[plane], widths[plane]);
            }
        }

    }

    if(transform_yuv2rgb(w, h, (const uint8_t**)&yuv_data, yuv_linesize, rgba_data, rgba_linesize, AV_PIX_FMT_YUV420P))
        s_player_callback(s_video_index, w, h, rgba_data[0]);*/

    if(transform_yuv2rgb(w, h, (const uint8_t**)&yuv_data, yuv_linesize, dst_data, dst_linesize, AV_PIX_FMT_YUV420P))
    {
        send_rbg_data(overlay, dst_data[0]);
    }


	av_freep(yuv_data);
	av_freep(dst_data);
}


static void deal_yuv420sp_data(SDL_VoutOverlay* overlay)
{
    if(!overlay)
        return;

    int w = overlay->pitches[0];
    int h = overlay->pitches[1];

    
	uint8_t* pixels[2];
	pixels[0] = overlay->pixels[0];
	pixels[1] = overlay->pixels[1];

	uint8_t *yuv_data[4];
	int yuv_linesize[4];

	uint8_t *dst_data[4];
	int dst_linesize[4];
    

	av_image_alloc(yuv_data, yuv_linesize, w, h, AV_PIX_FMT_NV12, 1);
	av_image_alloc(dst_data, dst_linesize, w, h, s_pix_format, 1);

	memcpy(yuv_data[0], pixels[0], w*h);
	memcpy(yuv_data[1], pixels[1], w*h/2);


    if(transform_yuv2rgb(w, h, (const uint8_t**)&yuv_data, yuv_linesize, dst_data, dst_linesize, AV_PIX_FMT_NV12))
    {
        send_rbg_data(overlay, dst_data[0]);
    }

	av_freep(yuv_data);
	av_freep(dst_data);

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
	av_log_set_level(AV_LOG_ERROR);
	
	s_media_player = ijkmp_android_create(media_player_msg_loop);
	ijkmp_set_option_int(s_media_player, IJKMP_OPT_CATEGORY_PLAYER, "mediacodec", 1);
	ijkmp_set_option_int(s_media_player, IJKMP_OPT_CATEGORY_PLAYER, "opensles", 1);
}

#endif


void h5_video_init(h5_video_callback video_func, h5_msg_callback msg_func, enum Pixel_Format format)
{
	s_player_callback = video_func;
	s_msg_callback = msg_func;

	switch(format)
    {
        case FMT_RGB:
            s_pix_format = AV_PIX_FMT_RGB24;
            break;
        case FMT_RGBA:
            s_pix_format = AV_PIX_FMT_RGBA;
            break;
        case FMT_ARGB:
            s_pix_format = AV_PIX_FMT_ARGB;
            break;
    }

	if (!s_global_init)
	{
		ijkmp_global_init();
    	msg_queue_init(&s_player_msg_queue);

    	s_global_init = true;
	}

}

//add function-------------------------------------------------------------------------------
//设置缓存地址，此为外部调用接口。
void  h5_video_set_cache_path(char* value) {
	if (cache_path) {
		free(cache_path);
		cache_path = NULL;
	}
	cache_path = malloc(sizeof(char) * 256);
	strncpy(cache_path, value, strlen(value) + 1);
}
//根据设置的缓存地址，设置播放的缓存option。
void  h5_video_set_cache_pathl(const char* url, char* value) {
	char* filename = strrchr(url, '/');
	char* filetype = strrchr(filename, '.');
	char cache_path[256];
	strncpy(cache_path, value, strlen(value) + 1);  //+1加上'\0'
	strncat(cache_path, filename, strlen(filename) - strlen(filetype));
	strcat(cache_path, CACHE_DATA_TYPE);
	printf("The cache file path is:%s\n", cache_path);
	ijkmp_set_option(s_media_player, IJKMP_OPT_CATEGORY_FORMAT, "cache_file_path", cache_path);

	strncpy(cache_path + strlen(cache_path) - strlen(CACHE_DATA_TYPE), CACHE_INDEX_TYPE, strlen(CACHE_INDEX_TYPE) + 1);
	printf("The cache index file path is:%s\n", cache_path);
	ijkmp_set_option(s_media_player, IJKMP_OPT_CATEGORY_FORMAT, "cache_map_path", cache_path);
	ijkmp_set_option_int(s_media_player, IJKMP_OPT_CATEGORY_FORMAT, "parse_cache_map", 1);
	ijkmp_set_option_int(s_media_player, IJKMP_OPT_CATEGORY_FORMAT, "auto_save_map", 1);
}
//删除h5_video_set_cache_path指定目录下的cache文件。
//若filename未指定则删除指定文件夹下所有缓存文件，spantime为已修改时间的最大范围，单位为秒，小于该值的不会被删除。
//filename指定则直接删除filename的缓存文件。
#ifdef __ANDROID__
void Traverse_path(char* dir, char** type,int nSpanTime) {
	char* curr_path = dir;
	DIR* dp;
	struct dirent* dirp;
	char tmp_path[300];	
	if ((dp = opendir(curr_path)) == NULL)
	{
		ALOGE("Traverse_path: Can not open this direction-- %s\n", curr_path);
		return;
	}
	if (type == NULL) {
		ALOGE("Traverse_path: Paragram type is null\n");
		return;
	}
	if (type[0]==NULL||strcmp(type[0],CACHE_INDEX_TYPE)!=0) {
		ALOGE("Traverse_path: Cache type[0] is not CACHE_INDEX_TYPE, but %s.\n", type[0]);
		return;
	}
	int current_length = strlen(curr_path);
	while ((dirp = readdir(dp)) != NULL)
	{
		if (dirp->d_type == 4)
		{
			if ((dirp->d_name)[0] == '.') //每个文件夹下都会有两个文件： '.'  和   '..'
				continue;

			sprintf(tmp_path, "%s/%s", curr_path, dirp->d_name);
			Traverse_path(tmp_path, type, nSpanTime);
		}
		else if (dirp->d_type == 8)
		{
			if (strcmp(dirp->d_name + strlen(dirp->d_name) - strlen(type[0]), type[0]) == 0)	//后缀比较。cai
			{
				sprintf(tmp_path,"%s/%s", curr_path, dirp->d_name);
				struct stat s;
				lstat(tmp_path, &s);
				if (time(NULL) - s.st_mtime >= nSpanTime) {
					//remove
					while (*type!=NULL) {
						MPTRACE("Deleted cache file is:%s\n", tmp_path);
						int ret = remove(tmp_path);//移除filename.cai filename.cad and the others.
						if (ret < 0) {
							ALOGE("Failed while deleting the file %s:%s", tmp_path, strerror(errno));
						}
						type++;
						if(*type)
							strncpy(tmp_path + strlen(tmp_path) - strlen(*(type - 1)), *type, strlen(*type) + 1);
					}
				}
			}
		}
	}
	closedir(dp);
}
#endif // __ANDROID__
void strrp(char* src, char sub, char rp)
{
	if (src == NULL)return;
	while (*src) {
		if (*src == sub) {
			*src = rp;
		}
		src++;
	}
}
void h5_video_destory_cache_all(int span_time) {
#ifdef _WINDOWS
	char command[300];
	int len = strlen(cache_path);
	char* cachePath = malloc(len + 1);

	if (cachePath == NULL) {
		ALOGE("h5_video_destory_cache_all: malloc of cachePath failed!");
		return;
	}

	strncpy(cachePath, cache_path, len + 1);
	strrp(cachePath, '/', '\\');
	sprintf(command, "del /s /q %s\\*%s", cachePath, CACHE_DATA_TYPE);
	system(command);
	MPTRACE("h5_video_destory_cache_all:system command is %s \n", command);//debug

	strncpy(command + strlen(command) - strlen(CACHE_DATA_TYPE), CACHE_INDEX_TYPE, strlen(CACHE_INDEX_TYPE));
	system(command);

	free(cachePath);
#elif __ANDROID__
	char* type[] = { CACHE_INDEX_TYPE,CACHE_DATA_TYPE,NULL };
	Traverse_path(cache_path, type, span_time);
#endif // _WINDOWS
}

void h5_video_destory_cache_filename(const char* filename) {
#ifdef _WINDOWS
	char command[300];
	int len = strlen(cache_path);
	char* cachePath = malloc(len + 100);

	if (cachePath == NULL) {
		ALOGE("h5_video_destory_cache_all: malloc of cachePath failed!");
		return;
	}

	sprintf(cachePath, "%s/%s", cache_path, filename);
	strrp(cachePath, '/', '\\');
	sprintf(command, "del /q  %s%s", cachePath, CACHE_DATA_TYPE);
	system(command);
	MPTRACE("h5_video_destory_cache_filename:system command is %s \n", command);//debug

	strncpy(command + strlen(command) - strlen(CACHE_DATA_TYPE), CACHE_INDEX_TYPE, strlen(CACHE_INDEX_TYPE));
	system(command);

	free(cachePath);
#elif __ANDROID__
	char* cachePath = malloc(strlen(cache_path) + 100);
	sprintf(cachePath, "%s/%s%s", cache_path, filename, CACHE_DATA_TYPE);
	int ret = remove(cachePath);
	if (ret < 0) {
		ALOGE("h5_video_destory_cache_filename:%s",strerror(errno));
	}
	strncpy(cachePath + strlen(cachePath) - strlen(CACHE_DATA_TYPE), CACHE_INDEX_TYPE, strlen(CACHE_INDEX_TYPE));
	remove(cachePath);
	free(cachePath);
#endif // _WINDOWS
}
void h5_video_destory_cache(const char* filename,int span_time) {
	if (cache_path == NULL || access(cache_path, 0) != 0) {
		ALOGE("h5_video_destory_cache: The cache path is not accessable!");
		return;
	}
	if (span_time < 0) {
		span_time = 0;
	}
	if (filename == NULL) {
		h5_video_destory_cache_all(span_time);
	}
	else {
		h5_video_destory_cache_filename(filename);
	}
}


//------------------------------------------------------------------------------------------------------

void  h5_video_play(const char* url, bool is_loop, int index)
{
	if (s_media_player)
	{
		ALOGE("h5_video_play: last video is not stopped, please call stop func!");
		return;
	}

#ifdef _WIN32
	create_window_player();
#elif TARGET_OS_MAC
	create_mac_player();
#elif TARGET_OS_IPHONE
	create_ios_player();

#elif __ANDROID__
	create_android_player();
#endif

	if (!s_media_player){
		ALOGE("created media object failed!");
		return;
	}

	ijkmp_set_option_int(s_media_player, IJKMP_OPT_CATEGORY_PLAYER, "start-on-prepared", 1);

	ijkmp_set_option_int(s_media_player, IJKMP_OPT_CATEGORY_PLAYER, "max-fps", 30);
	ijkmp_set_option_int(s_media_player, IJKMP_OPT_CATEGORY_PLAYER, "framedrop", 1);
	ijkmp_set_option_int(s_media_player, IJKMP_OPT_CATEGORY_PLAYER, "video-pictq-size", 3);

	ijkmp_set_option_int(s_media_player, IJKMP_OPT_CATEGORY_FORMAT, "reconnect", 1);
	ijkmp_set_option_int(s_media_player, IJKMP_OPT_CATEGORY_FORMAT, "timeout", 30 * 1000 * 1000);
	ijkmp_set_option(s_media_player, IJKMP_OPT_CATEGORY_FORMAT, "user-agent", "h5player");
	ijkmp_set_option_int(s_media_player, IJKMP_OPT_CATEGORY_FORMAT, "safe", 0);
	ijkmp_set_frame_callback(s_media_player, frame_update_callback);
	//add
	if (cache_path) {
		MPTRACE("Cache path is:%s\n",cache_path);
		h5_video_set_cache_pathl(url, cache_path);
	}
	s_video_index = index;
	// ijkio:cache:ffio:
	char protocol[100] = "ijkio:cache:ffio:";
	//char protocol[100] = "";
	strcat(protocol, url);
	
	ijkmp_set_data_source(s_media_player, protocol);
	ijkmp_prepare_async(s_media_player);    //~!!!!!
	msg_queue_start(&s_player_msg_queue);

	if (is_loop)
		ijkmp_set_loop(s_media_player, 100000);

	ijkmp_start(s_media_player);


}

void h5_video_set_speed(float speed) {
	if (!s_media_player)
		return;
	ijkmp_set_property_float(s_media_player, FFP_PROP_FLOAT_PLAYBACK_RATE, speed);
}

void h5_video_add_volume(float volume_changed) {
	if (!s_media_player)
		return;
	ijkmp_set_property_float(s_media_player, FFP_PROP_FLOAT_PLAYBACK_VOLUME, volume_changed + ijkmp_get_property_float(s_media_player, FFP_PROP_FLOAT_PLAYBACK_VOLUME, 0));
}
void h5_video_pause()
{
	if (!s_media_player)
        return;
	if (s_media_player->mp_state == MP_STATE_PAUSED) {
		ijkmp_start(s_media_player);
		return;
	}else 
		
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




void h5_video_seek(long sec)
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
    ijkmp_shutdown(s_media_player);   
    ijkmp_dec_ref_p(&s_media_player); 
    s_media_player = NULL;
}



void h5_video_destroy()
{
	if(s_global_init)
	{
		msg_queue_destroy(&s_player_msg_queue);
    	ijkmp_global_uninit();
    	s_global_init = false;
	}
	
}





void h5_video_set_option(const char* name, int value)
{
	if (!s_media_player)
        return;

    ijkmp_set_option_int(s_media_player, IJKMP_OPT_CATEGORY_FORMAT, name, value);
}

