#ifndef IJKPLAYER_H5_INTERFACE_PLAYER_H
#define IJKPLAYER_H5_INTERFACE_PLAYER_H

 #include <stdbool.h>

typedef  void (*h5_msg_callback)(int index, int event, int arg1, int arg2, const char* msg);

typedef  void(*h5_video_callback)(int index, int w, int h, unsigned char *data);

enum Pixel_Format {
	FMT_RGB,
	FMT_RGBA,
	FMT_ARGB,
};

#ifdef __cplusplus
extern "C"
{
#endif

	void            h5_video_init(h5_video_callback video_func, h5_msg_callback msg_func, enum Pixel_Format format);
	void            h5_video_set_option(const char* name, int value);
	void            h5_video_play(const char* url, bool is_loop, int index);
	void            h5_video_update();
	void            h5_video_pause();
	void            h5_video_resume();
	void 			h5_video_seek(int sec);
	void 			h5_video_stop();
	void 			h5_video_destroy();
	long			h5_video_get_duration();
	long			h5_video_get_position();

#ifdef __cplusplus
}
#endif

#endif


