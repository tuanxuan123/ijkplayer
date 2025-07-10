package com.vasd.pvideoplayer;


import android.util.Log;
import android.opengl.GLES30;
import android.graphics.SurfaceTexture;
import android.view.Surface;
import android.opengl.EGL14;
import android.opengl.EGLContext;






public class MediaTexture implements SurfaceTexture.OnFrameAvailableListener
{
    private static final int                    GL_TEXTURE_EXTERNAL_OES = 0x8D65;

    private static final int                    DEFUALT_WIDTH = 1280;
    private static final int                    DEFUALT_HEIGHT = 720;
    private static final String                 LOG_TAG = "PixVideo";
    /* 所有实例共用一份square，节省图形资源 */
    private static Square                       square = null;

    private SurfaceTexture                      surfaceTex = null;
    private Surface                             surface = null;
    private int                                 iTextureId = -1;
    private int                                 iFboTexId = -1;
    private int                                 iFboId = -1;
    private int                                 iWidth = DEFUALT_WIDTH;
    private int                                 iHeight = DEFUALT_HEIGHT;

    public  boolean                             bUpdateSurface = false;
    private float[]                             fSTMatirx = new float[16];



    public void UpdateTextureSize(int width, int height)
    {
        iWidth = width;
        iHeight = height;

        if(surfaceTex != null)
        {
            surfaceTex.setDefaultBufferSize(width, height);
        }

    }


    public Surface CreateSurface()
    {
        if(surface == null)
        {
            int[] texs = new int[1];

            GLES30.glGenTextures(1, texs, 0);
            iTextureId = texs[0];
            
            
            GLES30.glBindTexture(GL_TEXTURE_EXTERNAL_OES, iTextureId);
            GLES30.glTexParameterf(GL_TEXTURE_EXTERNAL_OES, GLES30.GL_TEXTURE_MIN_FILTER,
                GLES30.GL_LINEAR);
            GLES30.glTexParameterf(GL_TEXTURE_EXTERNAL_OES, GLES30.GL_TEXTURE_MAG_FILTER,
                GLES30.GL_LINEAR);
      
            GLES30.glTexParameteri(GL_TEXTURE_EXTERNAL_OES, GLES30.GL_TEXTURE_WRAP_S,
                GLES30.GL_CLAMP_TO_EDGE);
            GLES30.glTexParameteri(GL_TEXTURE_EXTERNAL_OES, GLES30.GL_TEXTURE_WRAP_T,
                GLES30.GL_CLAMP_TO_EDGE);
            
            surfaceTex = new SurfaceTexture(iTextureId);
            surfaceTex.setOnFrameAvailableListener(this);
            surfaceTex.setDefaultBufferSize(iWidth, iHeight);
            surface = new Surface(surfaceTex);
            
        }

        if (square == null)
        {
            square = new Square();
        }

        return surface;
    }


    private void createFBO()
    {
        if(iWidth <= 0 || iHeight <= 0)
        {
            iWidth = DEFUALT_WIDTH;
            iHeight = DEFUALT_HEIGHT;
        }

        Log.i(LOG_TAG,"createFBO ======================= width: " + iWidth + "  height: " + iHeight);
        
        int[] temp = new int[1];
        if(iFboId > 0)
        {
            temp[0] = iFboTexId;
            GLES30.glDeleteTextures(1, temp, 0);

            temp[0] = iFboId;
            GLES30.glDeleteFramebuffers(1, temp, 0);
        }


        GLES30.glGenFramebuffers(1, temp, 0);
        iFboId = temp[0];

        GLES30.glGenTextures(1, temp, 0);
        iFboTexId = temp[0];
        
        GLES30.glBindFramebuffer(GLES30.GL_FRAMEBUFFER, iFboId);
        GLES30.glBindTexture(GLES30.GL_TEXTURE_2D, iFboTexId);
        GLES30.glTexImage2D(GLES30.GL_TEXTURE_2D, 0, GLES30.GL_RGB, iWidth, iHeight, 0, GLES30.GL_RGB, GLES30.GL_UNSIGNED_BYTE, null);

        GLES30.glTexParameterf(GLES30.GL_TEXTURE_2D, GLES30.GL_TEXTURE_WRAP_S, GLES30.GL_CLAMP_TO_EDGE);
        GLES30.glTexParameterf(GLES30.GL_TEXTURE_2D, GLES30.GL_TEXTURE_WRAP_T, GLES30.GL_CLAMP_TO_EDGE);
        GLES30.glTexParameterf(GLES30.GL_TEXTURE_2D, GLES30.GL_TEXTURE_MAG_FILTER, GLES30.GL_LINEAR);
        GLES30.glTexParameterf(GLES30.GL_TEXTURE_2D, GLES30.GL_TEXTURE_MIN_FILTER, GLES30.GL_LINEAR);

        GLES30.glFramebufferTexture2D(GLES30.GL_FRAMEBUFFER, GLES30.GL_COLOR_ATTACHMENT0, GLES30.GL_TEXTURE_2D, iFboTexId, 0);
        GLES30.glBindTexture(GLES30.GL_TEXTURE_2D, 0);
        GLES30.glBindFramebuffer(GLES30.GL_FRAMEBUFFER, 0); 

    }




    //渲染线程
    public int UpdateTexture()
    {
        if(bUpdateSurface && surfaceTex != null)
        {
            bUpdateSurface = false;

            EGLContext eglContextRet = EGL14.eglGetCurrentContext();
            if (EGL14.EGL_NO_CONTEXT == eglContextRet)
            {
                Log.e(LOG_TAG, "UpdateTexture failed, CurrentContext is null");
                return 0;
            }

            if(iFboId < 0)
            {
                createFBO();
            }



            surfaceTex.updateTexImage();
            surfaceTex.getTransformMatrix(fSTMatirx);

            GLES30.glBindVertexArray(0);
            GLES30.glBindBuffer(GLES30.GL_ARRAY_BUFFER, 0);

            GLES30.glColorMask(true, true, true, true);
            GLES30.glBindFramebuffer(GLES30.GL_FRAMEBUFFER, iFboId);
            GLES30.glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
            GLES30.glClear(GLES30.GL_COLOR_BUFFER_BIT);
            GLES30.glDisable(GLES30.GL_DEPTH_TEST);
            GLES30.glDisable(GLES30.GL_STENCIL_TEST);
            GLES30.glDisable(GLES30.GL_CULL_FACE);
            GLES30.glDisable(GLES30.GL_BLEND);
            GLES30.glViewport(0, 0, iWidth, iHeight);

            square.draw(fSTMatirx, iTextureId);

            GLES30.glBindTexture(GLES30.GL_TEXTURE_2D, 0);
            GLES30.glBindFramebuffer(GLES30.GL_FRAMEBUFFER, 0);

            //Log.i(LOG_TAG, "UpdateTexture iFboTexId:" + iFboTexId);
            return iFboTexId;
        }

        return 0;
    }





    public void DestroySurface()
    {
        bUpdateSurface = false;


        if(iFboTexId > 0)
        {
            int[] texs = new int[1];

            texs[0] = iFboTexId;
            GLES30.glDeleteTextures(1, texs, 0);
            texs[0] = iFboId;
            GLES30.glDeleteFramebuffers(1, texs, 0);

            iFboTexId = -1;
            iFboId = -1;
        }

        if(surface != null)
        {
            int[] texs = new int[1];
            texs[0] = iTextureId;
            GLES30.glDeleteTextures(1, texs, 0);

            iTextureId = -1;
            surfaceTex = null;
            surface = null;
        }

        iWidth = 0;
        iHeight = 0;
    }



    @Override
    public void onFrameAvailable(SurfaceTexture surface) {
        if(surfaceTex == null)
            return;

        bUpdateSurface = true;
    }


}


