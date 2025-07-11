/*****************************************************************************
 * ijksdl_android.c
 *****************************************************************************
 *
 * Copyright (c) 2013 Bilibili
 * copyright (c) 2013 Zhang Rui <bbcallen@gmail.com>
 *
 * This file is part of ijkPlayer.
 *
 * ijkPlayer is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * ijkPlayer is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with ijkPlayer; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#include "ijksdl_android_jni.h"
#include "ijksdl_log.h"
#include <unistd.h>
#include <pthread.h>
#include <sys/system_properties.h>
#include "j4a/class/android/os/Build.h"

static JavaVM   *g_jvm = NULL;


static pthread_key_t g_thread_key;
static pthread_once_t g_key_once = PTHREAD_ONCE_INIT;
static bool is_in_atmos_blacklist = false;
static bool atmos_checked = false;


static const char *atmos_blacklist[] = {
    "VOG-AL00", "PCAM00", "PCHM30", "ELE-AL00", "PCDM10", "PCKM80", "PCLM10", 
    "SCM-W09", "LE2100", "PDCM00", "PCLM50", "PCKM00", "PCRM00", NULL
};

JavaVM *SDL_JNI_GetJvm()
{
    return g_jvm;
}

void SDL_JNI_SetJvm(JavaVM *vm)
{
    if(g_jvm)
        return;
    
    JNIEnv* env = NULL;

    g_jvm = vm;
    if ((*vm)->GetEnv(vm, (void**) &env, JNI_VERSION_1_6) != JNI_OK) {
        return;
    }

    J4A_LoadAll__catchAll(env);

    J4A_loadClass__J4AC_android_os_Build(env);
    J4A_loadClass__J4AC_AudioTrack(env);
}

static void SDL_JNI_ThreadDestroyed(void* value)
{
    JNIEnv *env = (JNIEnv*) value;
    if (env != NULL) {
        ALOGE("%s: [%d] didn't call SDL_JNI_DetachThreadEnv() explicity\n", __func__, (int)gettid());
        (*g_jvm)->DetachCurrentThread(g_jvm);
        pthread_setspecific(g_thread_key, NULL);
    }
}

static void make_thread_key()
{
    pthread_key_create(&g_thread_key, SDL_JNI_ThreadDestroyed);
}

jint SDL_JNI_SetupThreadEnv(JNIEnv **p_env)
{
    JavaVM *jvm = g_jvm;
    if (!jvm) {
        ALOGI("SDL_JNI_GetJvm: AttachCurrentThread: NULL jvm");
        return -1;
    }

    pthread_once(&g_key_once, make_thread_key);

    JNIEnv *env = (JNIEnv*) pthread_getspecific(g_thread_key);
    if (env) {
        *p_env = env;
        return 0;
    }

    if ((*jvm)->AttachCurrentThread(jvm, &env, NULL) == JNI_OK) {
        pthread_setspecific(g_thread_key, env);
        *p_env = env;
        return 0;
    }

    return -1;
}

void SDL_JNI_DetachThreadEnv(const char* thread_name)
{
    JavaVM *jvm = g_jvm;
    
    if(!jvm)
        return;

    ALOGI("%s: [%d], thread name: %s \n", __func__, (int)gettid(), thread_name ? thread_name : "NULL");

    pthread_once(&g_key_once, make_thread_key);

    JNIEnv *env = pthread_getspecific(g_thread_key);
    if (!env) {
        ALOGI("SDL_JNI_DetachThreadEnv not env: [%d]", (int)gettid());
        return;
    }
        
    pthread_setspecific(g_thread_key, NULL);

    if ((*jvm)->DetachCurrentThread(jvm) != JNI_OK) {
        ALOGE("DetachCurrentThread fail, id:%d", (int)gettid());
    }
        
}

int SDL_JNI_ThrowException(JNIEnv* env, const char* className, const char* msg)
{
    if ((*env)->ExceptionCheck(env)) {
        jthrowable exception = (*env)->ExceptionOccurred(env);
        (*env)->ExceptionClear(env);

        if (exception != NULL) {
            ALOGW("Discarding pending exception (%s) to throw", className);
            (*env)->DeleteLocalRef(env, exception);
        }
    }

    jclass exceptionClass = (*env)->FindClass(env, className);
    if (exceptionClass == NULL) {
        ALOGE("Unable to find exception class %s", className);
        /* ClassNotFoundException now pending */
        goto fail;
    }

    if ((*env)->ThrowNew(env, exceptionClass, msg) != JNI_OK) {
        ALOGE("Failed throwing '%s' '%s'", className, msg);
        /* an exception, most likely OOM, will now be pending */
        goto fail;
    }

    return 0;
fail:
    if (exceptionClass)
        (*env)->DeleteLocalRef(env, exceptionClass);
    return -1;
}

int SDL_JNI_ThrowIllegalStateException(JNIEnv *env, const char* msg)
{
    return SDL_JNI_ThrowException(env, "java/lang/IllegalStateException", msg);
}

jobject SDL_JNI_NewObjectAsGlobalRef(JNIEnv *env, jclass clazz, jmethodID methodID, ...)
{
    va_list args;
    va_start(args, methodID);

    jobject global_object = NULL;
    jobject local_object = (*env)->NewObjectV(env, clazz, methodID, args);
    if (!J4A_ExceptionCheck__throwAny(env) && local_object) {
        global_object = (*env)->NewGlobalRef(env, local_object);
        SDL_JNI_DeleteLocalRefP(env, &local_object);
    }

    va_end(args);
    return global_object;
}

void SDL_JNI_DeleteGlobalRefP(JNIEnv *env, jobject *obj_ptr)
{
    if (!obj_ptr || !*obj_ptr)
        return;

    (*env)->DeleteGlobalRef(env, *obj_ptr);
    *obj_ptr = NULL;
}

void SDL_JNI_DeleteLocalRefP(JNIEnv *env, jobject *obj_ptr)
{
    if (!obj_ptr || !*obj_ptr)
        return;

    (*env)->DeleteLocalRef(env, *obj_ptr);
    *obj_ptr = NULL;
}


int SDL_Android_GetApiLevel()
{
    static int SDK_INT = 0;
    if (SDK_INT > 0)
        return SDK_INT;


    char value[32] = {0};
    __system_property_get("ro.build.version.sdk", value);
    SDK_INT = atoi(value);
    return SDK_INT;

}



void SDL_JNI_AttachCurrentThread()
{
    if(!g_jvm)
        return;

    JNIEnv *env = NULL;
    if(JNI_OK != (*g_jvm)->AttachCurrentThread(g_jvm, &env, NULL)) {
        ALOGE("SDL_JNI_AttachCurrentThread failed!");
    }

}

void SDL_JNI_DetachCurrentThread()
{
    if(!g_jvm)
        return;

    if(JNI_OK != (*g_jvm)->DetachCurrentThread(g_jvm)) {
        ALOGE("SDL_JNI_DetachCurrentThread failed!");
    }
}


int SDL_Android_AudioSupportEAC3()
{
    if (SDL_Android_GetApiLevel() < IJK_API_28)
        return 0;

    if(atmos_checked) 
        return is_in_atmos_blacklist ? 0 : J4AC_MediaFormat__SupportEAC3();
    

    char device_model[128] = {0};

    __system_property_get("ro.product.model", device_model);

    int i = 0;

    while(atmos_blacklist[i]) {
        if(strcasecmp(atmos_blacklist[i], device_model) == 0) {
            is_in_atmos_blacklist = true;
            break;
        }
        ++i;
    }

    atmos_checked = true;

    return is_in_atmos_blacklist ? 0 : J4AC_MediaFormat__SupportEAC3();

}


jint JNI_OnLoad(JavaVM *vm, void *reserved)
{
    bool java_thread = true;
    JNIEnv* env = NULL;
    jint retval = JNI_ERR;
    g_jvm = vm;

    if ((*vm)->GetEnv(vm, (void**) &env, JNI_VERSION_1_6) == JNI_OK) {
        retval = JNI_VERSION_1_6;
        ALOGI("PixVideo JNI_OnLoad JNI_VERSION_1_6!");
    } else if ((*vm)->GetEnv(vm, (void**) &env, JNI_VERSION_1_4) == JNI_OK) {
        retval = JNI_VERSION_1_4;
        ALOGI("PixVideo JNI_OnLoad JNI_VERSION_1_4!");
    } else if ((*vm)->GetEnv(vm, (void**) &env, JNI_VERSION_1_2) == JNI_OK) {
        retval = JNI_VERSION_1_2;
        ALOGI("PixVideo JNI_OnLoad JNI_VERSION_1_2!");
    } else {
        ALOGI("PixVideo JNI_OnLoad GetEnv failed!");
        java_thread = false;
    }

    if (java_thread) {
        J4A_loadClass__J4AC_android_os_Build(env);
        J4A_loadClass__J4AC_AudioTrack(env);
    } else if ((*vm)->AttachCurrentThread(vm, &env, NULL) == JNI_OK) {
        J4A_loadClass__J4AC_android_os_Build(env);
        J4A_loadClass__J4AC_AudioTrack(env);

        (*vm)->DetachCurrentThread(vm);
    } else {
        ALOGE("PixVideo JNI_OnLoad failed!");
    }

    return retval;
}


