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
#include <jni.h>
#include "VulkanMain.hpp"
#include "h5_player.h"
#include <string>
static bool isPlaying = false;
void UpdateTextureData(int index, int w, int h, unsigned char *data)
{
    UpdateVulkanTexture(w, h, data);
}

void MessageCallback(int index, int event, int arg1, int arg2, const char *msg)
{
    __android_log_print(ANDROID_LOG_INFO, "VideoEvent", "MessageCallback event: %d, message: %s", event, msg);
}

//set the video cache path.
void SetCachePath(android_app* app) {
    //storage/emulated/0/Android/data/com.google.vulkan.videoplayer/files
    //We use SD card as the cache path first.
    char *ParentCachePath = NULL;
    if (app->activity->externalDataPath==NULL) {
        ParentCachePath = const_cast<char*>(app->activity->internalDataPath);
    } else {
        ParentCachePath = const_cast<char*>(app->activity->externalDataPath);
    }
    if(ParentCachePath==NULL){
        __android_log_print(ANDROID_LOG_ERROR, "SetCachePath",
                            "ParentCachePath is NULL, the cache function will be closed");
        return ;
    }
    char cache_path[256];
    strncpy(cache_path, app->activity->externalDataPath, strlen(ParentCachePath) + 1);
    //Editable cache path.
    char ChildCachePath[10] = "/cache";
    strncat(cache_path, ChildCachePath, strlen(ChildCachePath));
    h5_video_set_cache_path(cache_path);
    __android_log_print(ANDROID_LOG_ERROR, "SetCachePath",
                        "Final cache Path: %s", cache_path);
}

// Process the next main command.
void handle_cmd(android_app* app, int32_t cmd) {
    switch (cmd) {
        case APP_CMD_INIT_WINDOW: {
            // The window is being shown, get it ready.
            InitVulkan(app);

            h5_video_init(UpdateTextureData, MessageCallback);
            //set cache path and set cache option to true.
            SetCachePath(app);
            h5_video_play("https://image.smoba.qq.com/Video/playonline/Nobe_Video.mp4", false, 0);
            isPlaying = true;
            break;
        }
        case APP_CMD_TERM_WINDOW:
            // The window is being hidden or closed, clean it up.
            DeleteVulkan();

            break;
        case APP_CMD_PAUSE:
            h5_video_stop();
            __android_log_print(ANDROID_LOG_ERROR, "handle_cmd",
                                "APP_CMD_PAUSE");
            h5_video_destory_cache("Nobe_Video",10);
            break;
        default:
          __android_log_print(ANDROID_LOG_INFO, "Vulkan_VideoDemo",
                              "event not handled: %d", cmd);
    }
}
//add
void getCachePathAndPermissions(struct android_app* app){
    JNIEnv * env = NULL;
    app->activity->vm->AttachCurrentThread(&env, NULL);

    jclass native_activity_class = env->GetObjectClass(app->activity->clazz);
    jmethodID get_class_loader = env->GetMethodID(native_activity_class, "getClassLoader", "()Ljava/lang/ClassLoader;");
    jobject class_loader_object = env->CallObjectMethod(app->activity->clazz, get_class_loader);
    jclass class_loader = env->FindClass("java/lang/ClassLoader");
    jmethodID load_class = env->GetMethodID(class_loader, "loadClass", "(Ljava/lang/String;)Ljava/lang/Class;");

    jstring class_name = env->NewStringUTF("com.video.utils.FileUtils");
    jclass cls = jclass(env->CallObjectMethod(class_loader_object, load_class, class_name));
    //env->DeleteLocalRef(class_name);

    if(cls==NULL){
        __android_log_print(ANDROID_LOG_ERROR,"cls: ",
                            "NULL ERROR\n");
    }else{
        //jmethodID FileUtils_ = env->GetStaticMethodID( cls, "getCacheDirectory", "(Landroid/content/Context;Ljava/lang/String;)Ljava/lang/String");
        //env->CallStaticObjectMethod(cls,FileUtils_,env->get);
        __android_log_print(ANDROID_LOG_INFO,"handle_cmd",
                            "CallStaticObjectMethod\n");
        //申请写权限
        jmethodID FileUtils_ = env->GetStaticMethodID( cls, "verifyStoragePermissions", "(Landroid/app/Activity;)V");
        if(FileUtils_==NULL){
            __android_log_print(ANDROID_LOG_ERROR,"FileUtils_: ",
                                "NULL ERROR\n");
        }else {
            env->CallStaticVoidMethod(cls, FileUtils_, app->activity);
            jthrowable j_thr = env->ExceptionOccurred();
            if (j_thr) {
                __android_log_print(ANDROID_LOG_ERROR,"CallStaticVoidMethod: ",
                                    "ERROR\n");
                //清除异常
                env->ExceptionClear();
            }
        }
        env->DeleteLocalRef(cls);
    }

    app->activity->vm->DetachCurrentThread();
}
void android_main(struct android_app* app) {

  // Set the callback to process system events
  app->onAppCmd = handle_cmd;

  // Used to poll the events in the main loop
  int events;
  android_poll_source* source;

  // Main loop
  do {
    if (ALooper_pollAll(IsVulkanReady() ? 1 : 0, nullptr,
                        &events, (void**)&source) >= 0) {
      if (source != NULL) source->process(app, source);
    }

    // render if vulkan is ready
    if (IsVulkanReady()) {

      if(isPlaying)
        h5_video_update();

      VulkanDrawFrame();
    }
  } while (app->destroyRequested == 0);
  //add

  isPlaying = false;
}
