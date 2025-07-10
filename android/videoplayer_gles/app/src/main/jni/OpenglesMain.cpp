

#include "OpenglesMain.h"
//#include "../../../../../../../../Library/Android/sdk/ndk-bundle/sources/android/native_app_glue/android_native_app_glue.h"
//#include "../../../../../../../../Library/Android/sdk/ndk-bundle/platforms/android-24/arch-arm/usr/include/EGL/egl.h"
//#include "../../../../../../../../Library/Android/sdk/ndk-bundle/platforms/android-24/arch-arm/usr/include/GLES3/gl3.h"


#include <android/log.h>

#include <cassert>
#include <cstring>
#include <vector>



// Android log function wrappers
static const char* kTAG = "GLES_VideoDemo";
#define LOGI(...) \
  ((void)__android_log_print(ANDROID_LOG_INFO, kTAG, __VA_ARGS__))
#define LOGW(...) \
  ((void)__android_log_print(ANDROID_LOG_WARN, kTAG, __VA_ARGS__))
#define LOGE(...) \
  ((void)__android_log_print(ANDROID_LOG_ERROR, kTAG, __VA_ARGS__))


const char vShaderStr[] =
        "#version 300 es                            \n"
        "layout(location = 0) in vec4 a_position;   \n"
        "layout(location = 1) in vec2 a_texCoord;   \n"
        "out vec2 v_texCoord;                       \n"
        "void main()                                \n"
        "{                                          \n"
        "   gl_Position = a_position;               \n"
        "   v_texCoord = a_texCoord;                \n"
        "}                                          \n";


const char fShaderStr[] =
        "#version 300 es                                     \n"
        "precision mediump float;                            \n"
        "in vec2 v_texCoord;                                 \n"
        "layout(location = 0) out vec4 outColor;             \n"
        "uniform sampler2D videoMap;                        \n"
        "void main()                                         \n"
        "{                                                   \n"
        "                                                    \n"
        "  outColor = texture( videoMap, v_texCoord );       \n"
        "}                                                   \n";

static GLContext S_Context;
static bool      S_IsReady = false;

GLuint createTexture(int width, int height)
{
    GLuint texId;
    glGenTextures ( 1, &texId );
    glBindTexture ( GL_TEXTURE_2D, texId );

    glTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
    glTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
    glTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
    glTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );

    return texId;
}

GLuint loadShader(GLenum type, const char *shaderSrc )
{
    GLuint shader = glCreateShader ( type );
    glShaderSource ( shader, 1, &shaderSrc, NULL );
    glCompileShader ( shader );

    return shader;
}

GLuint loadProgram(const char* vShaderStr, const char* fShaderStr)
{
    GLuint vertexShader = loadShader(GL_VERTEX_SHADER, vShaderStr);
    GLuint fragmentShader = loadShader(GL_FRAGMENT_SHADER, fShaderStr);
    GLuint program = glCreateProgram();

    glAttachShader ( program, vertexShader );
    glAttachShader ( program, fragmentShader );

    glLinkProgram ( program );

    glDeleteShader ( vertexShader );
    glDeleteShader ( fragmentShader );
    return program;
}

void InitProgram(GLContext *context)
{
    context->program = loadProgram(vShaderStr, fShaderStr);
    context->videoTexLoc = glGetUniformLocation ( context->program, "videoMap" );

    for(int i = 0; i < VideoCount; ++i)
    {
        context->videoTexWidths[i] = 220;
        context->videoTexHeights[i] = 280;
        context->videoTexs[i] = createTexture(context->videoTexWidths[i], context->videoTexHeights[i]);
    }

    glClearColor ( 1.0f, 1.0f, 1.0f, 0.0f );
}

void InitOpengles(android_app* app)
{
    S_IsReady = false;

    EGLint mainVersion;
    EGLint minVersion;
    EGLConfig config;
    S_Context.displayType = EGL_DEFAULT_DISPLAY;
    S_Context.nativeWindow = app->window;

    S_Context.width = ANativeWindow_getWidth(S_Context.nativeWindow);
    S_Context.height = ANativeWindow_getHeight(S_Context.nativeWindow);

    S_Context.display = eglGetDisplay(S_Context.displayType);

    eglInitialize(S_Context.display, &mainVersion, &minVersion);


    EGLint configNum = 0;
    EGLint attributes[] = {
            EGL_RED_SIZE,       8,
            EGL_GREEN_SIZE,     8,
            EGL_BLUE_SIZE,      8,
            EGL_ALPHA_SIZE,     8,
            EGL_DEPTH_SIZE,     8,
            EGL_STENCIL_SIZE,   8,
            // if EGL_KHR_create_context extension is supported, then we will use
            // EGL_OPENGL_ES3_BIT_KHR instead of EGL_OPENGL_ES2_BIT in the attribute list
            EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
            EGL_NONE

    };
    eglChooseConfig(S_Context.display, attributes, &config, 1, &configNum);

    EGLint format = 0;
    eglGetConfigAttrib(S_Context.display, config, EGL_NATIVE_VISUAL_ID, &format);
    ANativeWindow_setBuffersGeometry(S_Context.nativeWindow, 0, 0, format);

    EGLint contextAttribs[] = { EGL_CONTEXT_CLIENT_VERSION, 3, EGL_NONE };

    S_Context.surface = eglCreateWindowSurface(S_Context.display, config, S_Context.nativeWindow, NULL);
    S_Context.context = eglCreateContext(S_Context.display, config, EGL_NO_CONTEXT, contextAttribs);

    eglMakeCurrent(S_Context.display, S_Context.surface, S_Context.surface, S_Context.context);

    InitProgram(&S_Context);

    S_IsReady = true;
}



void DeleteOpengles(void)
{
    S_IsReady = false;
    glDeleteProgram ( S_Context.program);

}


bool IsOpenglesReady(void)
{
    return S_IsReady;
}


static GLfloat vVertices[VideoCount][20] = {
        {-0.98f,  0.9f, 0.0f, 0.0f,  0.0f,// Position 0 TexCoord 0
         -0.98f,  0.1f, 0.0f, 0.0f,  1.0f,// Position 1 TexCoord 1
         -0.68f,  0.1f, 0.0f, 1.0f,  1.0f,
         -0.68f,  0.9f, 0.0f, 1.0f,  0.0f},

        {-0.6f,  0.9f, 0.0f, 0.0f,  0.0f,// Position 0 TexCoord 0
         -0.6f,  0.1f, 0.0f, 0.0f,  1.0f,// Position 1 TexCoord 1
         -0.3f,  0.1f, 0.0f, 1.0f,  1.0f,
         -0.3f,  0.9f, 0.0f, 1.0f,  0.0f},

        {-0.2f,  0.9f, 0.0f, 0.0f,  0.0f,// Position 0 TexCoord 0
         -0.2f,  0.1f, 0.0f, 0.0f,  1.0f,// Position 1 TexCoord 1
          0.1f,  0.1f, 0.0f, 1.0f,  1.0f,
          0.1f,  0.9f, 0.0f, 1.0f,  0.0f},

        { 0.2f,  0.9f, 0.0f, 0.0f,  0.0f,// Position 0 TexCoord 0
          0.2f,  0.1f, 0.0f, 0.0f,  1.0f,// Position 1 TexCoord 1
          0.5f,  0.1f, 0.0f, 1.0f,  1.0f,
          0.5f,  0.9f, 0.0f, 1.0f,  0.0f},

        { 0.6f,  0.9f, 0.0f, 0.0f,  0.0f,// Position 0 TexCoord 0
          0.6f,  0.1f, 0.0f, 0.0f,  1.0f,// Position 1 TexCoord 1
          0.9f,  0.1f, 0.0f, 1.0f,  1.0f,
          0.9f,  0.9f, 0.0f, 1.0f,  0.0f},

        {-0.98f,  -0.1f, 0.0f, 0.0f,  0.0f,// Position 0 TexCoord 0
         -0.98f,  -0.9f, 0.0f, 0.0f,  1.0f,// Position 1 TexCoord 1
         -0.68f,  -0.9f, 0.0f, 1.0f,  1.0f,
         -0.68f,  -0.1f, 0.0f, 1.0f,  0.0f},

        {-0.6f,  -0.1f, 0.0f, 0.0f,  0.0f,// Position 0 TexCoord 0
         -0.6f,  -0.9f, 0.0f, 0.0f,  1.0f,// Position 1 TexCoord 1
         -0.3f,  -0.9f, 0.0f, 1.0f,  1.0f,
         -0.3f,  -0.1f, 0.0f, 1.0f,  0.0f},

        {-0.2f,  -0.1f, 0.0f, 0.0f,  0.0f,// Position 0 TexCoord 0
         -0.2f,  -0.9f, 0.0f, 0.0f,  1.0f,// Position 1 TexCoord 1
          0.1f,  -0.9f, 0.0f, 1.0f,  1.0f,
          0.1f,  -0.1f, 0.0f, 1.0f,  0.0f},

        { 0.2f,  -0.1f, 0.0f, 0.0f,  0.0f,// Position 0 TexCoord 0
          0.2f,  -0.9f, 0.0f, 0.0f,  1.0f,// Position 1 TexCoord 1
          0.5f,  -0.9f, 0.0f, 1.0f,  1.0f,
          0.5f,  -0.1f, 0.0f, 1.0f,  0.0f},

        { 0.6f,  -0.1f, 0.0f, 0.0f,  0.0f,// Position 0 TexCoord 0
          0.6f,  -0.9f, 0.0f, 0.0f,  1.0f,// Position 1 TexCoord 1
          0.9f,  -0.9f, 0.0f, 1.0f,  1.0f,
          0.9f,  -0.1f, 0.0f, 1.0f,  0.0f},

};

void DrawVideoFrame(void)
{
/*
    GLfloat vVertices[2][20] = {
                            {-0.95f,  0.35f, 0.0f,  // Position 0
                             0.0f,  0.0f,        // TexCoord 0
                             -0.95f, -0.35f, 0.0f,  // Position 1
                             0.0f,  1.0f,        // TexCoord 1
                             -0.05f, -0.35f, 0.0f,  // Position 2
                             1.0f,  1.0f,        // TexCoord 2
                             -0.05f,  0.35f, 0.0f,  // Position 3
                             1.0f,  0.0f},         // TexCoord 3

                            {0.05f,  0.35f, 0.0f,  // Position 0
                             0.0f,  0.0f,        // TexCoord 0
                             0.05f, -0.35f, 0.0f,  // Position 1
                             0.0f,  1.0f,        // TexCoord 1
                             0.95f, -0.35f, 0.0f,  // Position 2
                             1.0f,  1.0f,        // TexCoord 2
                             0.95f,  0.35f, 0.0f,  // Position 3
                             1.0f,  0.0f},


    };*/


    GLushort indices[] = { 0, 1, 2, 0, 2, 3 };

    glViewport ( 0, 0, S_Context.width, S_Context.height );
    glClear ( GL_COLOR_BUFFER_BIT );

    for(int i = 0; i < VideoCount; ++i)
    {
        glUseProgram ( S_Context.program);

        glVertexAttribPointer ( 0, 3, GL_FLOAT,
                                GL_FALSE, 5 * sizeof ( GLfloat ), vVertices[i] );
        // Load the texture coordinate
        glVertexAttribPointer ( 1, 2, GL_FLOAT,
                                GL_FALSE, 5 * sizeof ( GLfloat ), &vVertices[i][3] );

        glEnableVertexAttribArray ( 0 );
        glEnableVertexAttribArray ( 1 );

        glActiveTexture ( GL_TEXTURE0 );
        glBindTexture ( GL_TEXTURE_2D, S_Context.videoTexs[i]);
        glUniform1i ( S_Context.videoTexLoc, 0 );

        glDrawElements ( GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, indices );
    }

    eglSwapBuffers(S_Context.display, S_Context.surface);
}


void UpdateVideoTexture(int w, int h, unsigned char *data, int index, Pixel_Format format)
{
    if (w != S_Context.videoTexWidths[index] || h != S_Context.videoTexHeights[index])
    {
        GLuint texs[1];
        texs[0] = S_Context.videoTexs[index];
        glDeleteTextures(1, texs);

        S_Context.videoTexWidths[index] = w;
        S_Context.videoTexHeights[index] = h;
        S_Context.videoTexs[index] = createTexture(w, h);

    }

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glBindTexture(GL_TEXTURE_2D, S_Context.videoTexs[index]);

    if(format == FMT_RGB)
        glTexImage2D ( GL_TEXTURE_2D, 0, GL_RGB, w, h, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
    else
        glTexImage2D ( GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);

}