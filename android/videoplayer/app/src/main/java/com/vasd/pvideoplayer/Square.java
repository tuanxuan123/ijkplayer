
package com.vasd.pvideoplayer;

import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.nio.FloatBuffer;
import android.opengl.GLES30;
import android.util.Log;

public class Square {
    private static final int                    GL_TEXTURE_EXTERNAL_OES = 0x8D65;
    private static final String                 LOG_TAG = "PixVideo";

    private final String vertexShaderCode =
            "uniform mat4 uSTMatrix;\n"  +
                    "attribute vec2 vPosition;\n" +
                    "attribute vec2 aTexCoord;\n" +
                    "varying vec2 vTexCoord;\n" +
                    "void main() {\n" +
                    "gl_Position = vec4(vPosition, 0, 1);\n" +
                    "vTexCoord = (uSTMatrix * vec4(aTexCoord, 0, 1)).xy;\n" +
                    "}\n";

    private final String fragmentShaderCode =

            "#extension GL_OES_EGL_image_external : require\n" +
                    "precision mediump float;\n" +
                    "uniform samplerExternalOES sTexture;\n" +
                    "varying vec2 vTexCoord;\n" +
                    "void main() {\n" +
                    "gl_FragColor = texture2D(sTexture, vTexCoord);\n" +
                    "}\n";


    private FloatBuffer vertexBuffer;
    private FloatBuffer texCoordBuffer;

    private int mProgram;
    private int mPositionHandle;
    private int mTexHandle;
    private int mSTMatrixHandle;
    private int mSTextureHandle;


    static final float positionData[] = {
            -1.0f, -1.0f, 0.0f,
            1.0f, -1.0f,  0.0f,
            -1.0f, 1.0f,  0.0f,
            1.0f, 1.0f, 0.0f
    };

    static final float texCoordData[] = {
            0.0f, 1.0f,
            1.0f, 1.0f,
            0.0f, 0.0f,
            1.0f, 0.0f
    };


    public Square() {
        ByteBuffer bb = ByteBuffer.allocateDirect(positionData.length * 4);
        bb.order(ByteOrder.nativeOrder());
        vertexBuffer = bb.asFloatBuffer();
        vertexBuffer.put(positionData);
        vertexBuffer.position(0);

        ByteBuffer tcd = ByteBuffer.allocateDirect(texCoordData.length * 4);
        tcd.order(ByteOrder.nativeOrder());
        texCoordBuffer = tcd.asFloatBuffer();
        texCoordBuffer.put(texCoordData);
        texCoordBuffer.position(0);


        int vertexShader = loadShader(
                GLES30.GL_VERTEX_SHADER,
                vertexShaderCode);
        int fragmentShader = loadShader(
                GLES30.GL_FRAGMENT_SHADER,
                fragmentShaderCode);

        mProgram = GLES30.glCreateProgram();
        GLES30.glAttachShader(mProgram, vertexShader);
        GLES30.glAttachShader(mProgram, fragmentShader);


        GLES30.glLinkProgram(mProgram);
        int[] status = new int[1];
        GLES30.glGetProgramiv(mProgram, GLES30.GL_LINK_STATUS, status, 0);

        if(status[0] != GLES30.GL_TRUE)
        {
            Log.e(LOG_TAG,"glGetProgramiv is error ===================== ");
        }

        mPositionHandle = GLES30.glGetAttribLocation(mProgram, "vPosition");
        mTexHandle = GLES30.glGetAttribLocation(mProgram, "aTexCoord");
        mSTMatrixHandle = GLES30.glGetUniformLocation(mProgram, "uSTMatrix");
        mSTextureHandle = GLES30.glGetUniformLocation(mProgram, "sTexture");
    }

    public void deleteProgram()
    {
        GLES30.glDeleteProgram(mProgram);
    }

    public int loadShader(int type, String shaderCode){

        int shader = GLES30.glCreateShader(type);

        GLES30.glShaderSource(shader, shaderCode);
        checkGlError("glShaderSource");
        GLES30.glCompileShader(shader);
        int[] compileStatus = new int[1];
        GLES30.glGetShaderiv(shader, GLES30.GL_COMPILE_STATUS, compileStatus, 0);

        if(compileStatus[0] == 0)
        {
            Log.e(LOG_TAG,"compileStatus is error ===================== "+type);
        }
        checkGlError("glCompileShader");
        return shader;
    }

    public static void checkGlError(String glOperation) {
        int error;
        while ((error = GLES30.glGetError()) != GLES30.GL_NO_ERROR) {
            Log.e(LOG_TAG, glOperation + ": glError " + error);
            //throw new RuntimeException(glOperation + ": glError " + error);
        }
    }


    public void draw(float[] stMatirx, int texId) {

        GLES30.glUseProgram(mProgram);


        GLES30.glActiveTexture(GLES30.GL_TEXTURE0);
        GLES30.glBindTexture(GL_TEXTURE_EXTERNAL_OES, texId);

        GLES30.glUniform1i(mSTextureHandle, 0);
        GLES30.glUniformMatrix4fv(mSTMatrixHandle, 1, false, stMatirx, 0);

        GLES30.glEnableVertexAttribArray(mPositionHandle);
        GLES30.glVertexAttribPointer(
                mPositionHandle, 3,
                GLES30.GL_FLOAT, false,
                0, vertexBuffer);

        GLES30.glEnableVertexAttribArray(mTexHandle);
        GLES30.glVertexAttribPointer(
                mTexHandle, 2,
                GLES30.GL_FLOAT, false,
                0, texCoordBuffer);
        GLES30.glDrawArrays(GLES30.GL_TRIANGLE_STRIP, 0, 4);
    }

}