// Copyright 2016 Google Inc. All Rights Reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
#include <android/log.h>
#include <android_native_app_glue.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "OpenglesMain.h"
#include "h5_player.h"
#include "pandora_player.h"
#include "pxauth/pxauth.h"

#include <media/NdkMediaCodec.h>
#include <media/NdkMediaFormat.h>
#include <media/NdkMediaExtractor.h>

const char* filename = "/storage/emulated/0/Android/data/com.video.demo/files/test.mp4";//"http://10.64.215.141:8082/video/Atmos_stereo.mp4";
const char* filename1 = "https://image.smoba.qq.com/Video/playonline/Nobe_Video.mp4";

static bool isPlaying = false;

static int videoTags[2] = {0};
static Pixel_Format videoTexFormat = FMT_RGB;

void UpdateTextureData(int w, int h, unsigned char *data, int tag)
{
    UpdateVideoTexture(w, h, data, tag, videoTexFormat);
}

void PandoraUpdateTextureData(uint8_t* data[], int w, int h, int format, int tag)
{
    UpdateVideoTexture(w, h, data[tag], tag, static_cast<Pixel_Format>(format));
}

void MessageCallback(int event, int arg1, int arg2, const char *msg, int tag)
{
    //__android_log_print(ANDROID_LOG_INFO, "VideoEvent", "MessageCallback event: %d, message: %s", event, msg);
}


static bool isPause = false;
void timer_func(int sig)
{

    /*if(isPause){
        isPause = false;
        h5_video_resume(videoTags[0]);
        h5_video_resume(videoTags[1]);
        alarm(4);
    }else{
        isPause = true;
        h5_video_pause(videoTags[0]);
        h5_video_pause(videoTags[1]);
        alarm(2);
    }
    //h5_video_stop();
    //isPlaying = false;*/
    //h5_video_seek(2, videoTags[0]);
    h5_video_set_rate(1.5f, videoTags[0]);
}



// Process the next main command.
void handle_cmd(android_app* app, int32_t cmd) {
  switch (cmd) {
    case APP_CMD_INIT_WINDOW:
      InitOpengles(app);

      if(!isPlaying){
          //videoTags[0] = h5_video_play(filename, true);
          //videoTags[1] = h5_video_play(filename1, true);

          for(int i = 0; i < 1; ++i)
              videoTags[0] = h5_video_play(filename1, 1, 0);

          for(int i = 1; i < 2; ++i)
              videoTags[1] = pandora_video_play(filename1, 1);
          isPlaying = true;
      }


      break;
    case APP_CMD_TERM_WINDOW:
        DeleteOpengles();
      break;
    default:
      __android_log_print(ANDROID_LOG_INFO, "Opengles_VideoDemo",
                          "event not handled: %d", cmd);
  }
}

void android_main(struct android_app* app) {

  // Set the callback to process system events
  app->onAppCmd = handle_cmd;

  // Used to poll the events in the main loop
  int events;
  android_poll_source* source;






    JavaVM *jvm = app->activity->vm;
    JNIEnv *env = NULL;

    if(JNI_OK != jvm->AttachCurrentThread(&env, NULL)) {
       return;
    }
    pxa_init_android(jvm);
    //h5_video_set_jvm(jvm);

    AAsset* asset_file = AAssetManager_open(app->activity->assetManager, "Atmos_stereo.mp4", AASSET_MODE_BUFFER);
    size_t fileLength = AAsset_getLength(asset_file);


    char *buffer = (char*) malloc(fileLength+1);
    buffer[fileLength] = 0;
    AAsset_read(asset_file, buffer, fileLength);
    AAsset_close(asset_file);

    FILE* file = fopen(filename, "w");
    fwrite(buffer, fileLength, 1, file);

    fclose(file);

    signal(SIGALRM, timer_func);
    alarm(2);

  h5_video_init(UpdateTextureData, MessageCallback, videoTexFormat);
    //PandoraUpdateTextureData
  pandora_video_init(PandoraUpdateTextureData, MessageCallback);

  // Main loop
  do {
    if (ALooper_pollAll(IsOpenglesReady() ? 1 : 0, NULL,
                        &events, (void**)&source) >= 0) {
      if (source != NULL) source->process(app, source);
    }

    // render if  ready
    if (IsOpenglesReady()) {

      if(isPlaying)
      {
          for(int i = 0; i < 10; ++i)
          {
              h5_video_update(i);
              pandora_video_update(i);
          }
      }

      DrawVideoFrame();
    }
  } while (app->destroyRequested == 0);

  isPlaying = false;
}
