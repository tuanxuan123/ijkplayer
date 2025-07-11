package com.vasd.pvideoplayer;

import android.view.Surface;
import android.util.Log;







public class MediaTextureManager
{
    private static final int                    MAX_TEXTURE_COUNT = 16;
    private static final int                    PLAYER_COUNT = 2;
    private static final String                 LOG_TAG = "PixVideo";

    private static MediaTexture[][]               s_textureList = new MediaTexture[MAX_TEXTURE_COUNT][PLAYER_COUNT];

    static
    {
        System.loadLibrary("PixVideo");
        System.loadLibrary("videotexture");
    }



    public static native void SetSurface(Surface surface1, Surface surface2, int tag);


    public static void UpdateTextureSize(int width, int height, int tag, int curIdx)
    {
        if(tag >= MAX_TEXTURE_COUNT || curIdx > 1)
        {
            Log.e(LOG_TAG, "error: tag is bigger than MAX_VIDEO_COUNT!");
            return;
        }

        if(s_textureList[tag][curIdx] != null)
        {
            s_textureList[tag][curIdx].UpdateTextureSize(width, height);
        }
    }

    public static void CreateSurface(int tag)
    {
        if(tag >= MAX_TEXTURE_COUNT)
        {
            Log.e(LOG_TAG, "error: tag is bigger than MAX_VIDEO_COUNT!");
            return;
        }

        if(s_textureList[tag][0] == null)
        {
            s_textureList[tag][0] = new MediaTexture();
            s_textureList[tag][1] = new MediaTexture();
        }

        if(s_textureList[tag][0] != null)
        {
            Surface sf1 = s_textureList[tag][0].CreateSurface();
            Surface sf2 = s_textureList[tag][1].CreateSurface();
            SetSurface(sf1, sf2, tag);
        }
    }



    //渲染线程
    public static int UpdateTexture(int tag, int currentIdx)
    {
        if (tag >= MAX_TEXTURE_COUNT)
        {
            Log.e(LOG_TAG, "error: tag is bigger than MAX_VIDEO_COUNT!");
            return 0;
        }

        if (s_textureList[tag][0] != null)
        {
            int tex1 = s_textureList[tag][0].UpdateTexture();
            int tex2 = s_textureList[tag][1].UpdateTexture();

            //Log.i(LOG_TAG, "UpdateTexture, currentIdx: " + currentIdx + " tex1:" + tex1 + "  tex2:" + tex2);
            return currentIdx == 0 ? tex1 : tex2;
        }

        return 0;
    }


    public static void DestroySurface(int tag)
    {
        if (tag >= MAX_TEXTURE_COUNT)
        {
            Log.e(LOG_TAG, "error: tag is bigger than MAX_VIDEO_COUNT!");
            return;
        }

        for (int i = 0; i < PLAYER_COUNT; ++i)
        {
            if (s_textureList[tag][i] != null)
            {
                s_textureList[tag][i].DestroySurface();
                s_textureList[tag][i] = null;
            }
        }
    }

    
}


