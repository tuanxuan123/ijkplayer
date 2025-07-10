
#ifndef __OPENGLES_MAIN_H__
#define __OPENGLES_MAIN_H__

#include <android_native_app_glue.h>
#include <GLES3/gl3.h>
#include <EGL/egl.h>
#include <EGL/eglext.h>

#include "h5_player.h"

#define  VideoCount         10


struct GLContext
{
    GLint width;
    GLint height;
    EGLNativeDisplayType  displayType;
    EGLNativeWindowType   nativeWindow;
    EGLDisplay            display;
    EGLContext            context;
    EGLSurface            surface;
    GLuint                program;
    GLuint                videoTexs[VideoCount];
    GLint                 videoTexLoc;

    int                   videoTexWidths[VideoCount];
    int                   videoTexHeights[VideoCount];
};


void InitOpengles(android_app* app);

void DeleteOpengles(void);

bool IsOpenglesReady(void);

//Render a frame
void DrawVideoFrame(void);


void UpdateVideoTexture(int w, int h, unsigned char *data, int index, Pixel_Format format);

#endif


