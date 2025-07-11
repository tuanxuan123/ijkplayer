/*
 * Copyright (C) 2015 Zhang Rui <bbcallen@gmail.com>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/*
 * https://github.com/Bilibili/jni4android
 * This file is automatically generated by jni4android, do not modify.
 */

#ifndef J4A__android_media_MediaFormat__H
#define J4A__android_media_MediaFormat__H

#include "j4a/j4a_base.h"

jobject J4AC_android_media_MediaFormat__MediaFormat(JNIEnv *env);
jobject J4AC_android_media_MediaFormat__MediaFormat__catchAll(JNIEnv *env);
jobject J4AC_android_media_MediaFormat__MediaFormat__asGlobalRef__catchAll(JNIEnv *env);
jobject J4AC_android_media_MediaFormat__createVideoFormat(JNIEnv *env, jstring mime, jint width, jint height);
jobject J4AC_android_media_MediaFormat__createVideoFormat__catchAll(JNIEnv *env, jstring mime, jint width, jint height);
jobject J4AC_android_media_MediaFormat__createVideoFormat__asGlobalRef__catchAll(JNIEnv *env, jstring mime, jint width, jint height);
jobject J4AC_android_media_MediaFormat__createVideoFormat__withCString(JNIEnv *env, const char *mime_cstr__, jint width, jint height);
jobject J4AC_android_media_MediaFormat__createVideoFormat__withCString__catchAll(JNIEnv *env, const char *mime_cstr__, jint width, jint height);
jobject J4AC_android_media_MediaFormat__createVideoFormat__withCString__asGlobalRef__catchAll(JNIEnv *env, const char *mime_cstr__, jint width, jint height);
jint J4AC_android_media_MediaFormat__getInteger(JNIEnv *env, jobject thiz, jstring name);
jint J4AC_android_media_MediaFormat__getInteger__catchAll(JNIEnv *env, jobject thiz, jstring name);
jint J4AC_android_media_MediaFormat__getInteger__withCString(JNIEnv *env, jobject thiz, const char *name_cstr__);
jint J4AC_android_media_MediaFormat__getInteger__withCString__catchAll(JNIEnv *env, jobject thiz, const char *name_cstr__);
void J4AC_android_media_MediaFormat__setInteger(JNIEnv *env, jobject thiz, jstring name, jint value);
void J4AC_android_media_MediaFormat__setInteger__catchAll(JNIEnv *env, jobject thiz, jstring name, jint value);
void J4AC_android_media_MediaFormat__setInteger__withCString(JNIEnv *env, jobject thiz, const char *name_cstr__, jint value);
void J4AC_android_media_MediaFormat__setInteger__withCString__catchAll(JNIEnv *env, jobject thiz, const char *name_cstr__, jint value);
void J4AC_android_media_MediaFormat__setByteBuffer(JNIEnv *env, jobject thiz, jstring name, jobject bytes);
void J4AC_android_media_MediaFormat__setByteBuffer__catchAll(JNIEnv *env, jobject thiz, jstring name, jobject bytes);
void J4AC_android_media_MediaFormat__setByteBuffer__withCString(JNIEnv *env, jobject thiz, const char *name_cstr__, jobject bytes);
void J4AC_android_media_MediaFormat__setByteBuffer__withCString__catchAll(JNIEnv *env, jobject thiz, const char *name_cstr__, jobject bytes);
int J4A_loadClass__J4AC_android_media_MediaFormat(JNIEnv *env);
int J4AC_android_MediaFormat__SupportEAC3();

#define J4A_HAVE_SIMPLE__J4AC_android_media_MediaFormat

#define J4AC_MediaFormat__MediaFormat J4AC_android_media_MediaFormat__MediaFormat
#define J4AC_MediaFormat__MediaFormat__asGlobalRef__catchAll J4AC_android_media_MediaFormat__MediaFormat__asGlobalRef__catchAll
#define J4AC_MediaFormat__MediaFormat__catchAll J4AC_android_media_MediaFormat__MediaFormat__catchAll
#define J4AC_MediaFormat__createVideoFormat J4AC_android_media_MediaFormat__createVideoFormat
#define J4AC_MediaFormat__createVideoFormat__asGlobalRef__catchAll J4AC_android_media_MediaFormat__createVideoFormat__asGlobalRef__catchAll
#define J4AC_MediaFormat__createVideoFormat__catchAll J4AC_android_media_MediaFormat__createVideoFormat__catchAll
#define J4AC_MediaFormat__createVideoFormat__withCString J4AC_android_media_MediaFormat__createVideoFormat__withCString
#define J4AC_MediaFormat__createVideoFormat__withCString__asGlobalRef__catchAll J4AC_android_media_MediaFormat__createVideoFormat__withCString__asGlobalRef__catchAll
#define J4AC_MediaFormat__createVideoFormat__withCString__catchAll J4AC_android_media_MediaFormat__createVideoFormat__withCString__catchAll
#define J4AC_MediaFormat__getInteger J4AC_android_media_MediaFormat__getInteger
#define J4AC_MediaFormat__getInteger__catchAll J4AC_android_media_MediaFormat__getInteger__catchAll
#define J4AC_MediaFormat__getInteger__withCString J4AC_android_media_MediaFormat__getInteger__withCString
#define J4AC_MediaFormat__getInteger__withCString__catchAll J4AC_android_media_MediaFormat__getInteger__withCString__catchAll
#define J4AC_MediaFormat__setInteger J4AC_android_media_MediaFormat__setInteger
#define J4AC_MediaFormat__setInteger__catchAll J4AC_android_media_MediaFormat__setInteger__catchAll
#define J4AC_MediaFormat__setInteger__withCString J4AC_android_media_MediaFormat__setInteger__withCString
#define J4AC_MediaFormat__setInteger__withCString__catchAll J4AC_android_media_MediaFormat__setInteger__withCString__catchAll
#define J4AC_MediaFormat__setByteBuffer J4AC_android_media_MediaFormat__setByteBuffer
#define J4AC_MediaFormat__setByteBuffer__catchAll J4AC_android_media_MediaFormat__setByteBuffer__catchAll
#define J4AC_MediaFormat__setByteBuffer__withCString J4AC_android_media_MediaFormat__setByteBuffer__withCString
#define J4AC_MediaFormat__setByteBuffer__withCString__catchAll J4AC_android_media_MediaFormat__setByteBuffer__withCString__catchAll
#define J4A_loadClass__J4AC_MediaFormat J4A_loadClass__J4AC_android_media_MediaFormat
#define J4AC_MediaFormat__SupportEAC3 J4AC_android_MediaFormat__SupportEAC3

#endif//J4A__android_media_MediaFormat__H
