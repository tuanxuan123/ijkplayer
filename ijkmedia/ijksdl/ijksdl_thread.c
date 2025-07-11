/*****************************************************************************
 * ijksdl_thread.c
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

#include <errno.h>
#include <string.h>
#include "ijksdl_inc_internal.h"
#include "ijksdl_thread.h"
#ifdef __ANDROID__
#include "ijksdl/android/ijksdl_android_jni.h"
#endif

#ifndef _WIN32
#include <unistd.h>
#endif


#if !defined(__APPLE__)
// using ios implement for autorelease
static void *SDL_RunThread(void *data)
{
    SDL_Thread *thread = data;
    
#ifndef _WIN32
	ALOGI("SDL_RunThread: [%d] %s\n", (int)gettid(), thread->name);
    const int kMaxThreadNameLen = 16;
    char threadName[kMaxThreadNameLen];
    /* 确保不会超过目标数组大小kMaxThreadNameLen，并复制字符串 */
    strncpy(threadName, thread->name, sizeof(threadName) - 1);
    threadName[sizeof(threadName) - 1] = '\0'; // 确保以 null 结尾

    int ret = pthread_setname_np(pthread_self(), threadName);
    if (ret != 0)
    {
        ALOGE("SDL_RunThread: [%d] set thread name %s failed, ret=%d\n", (int)gettid(), thread->name, ret);
    }
    else
    {
        ALOGI("SDL_RunThread: [%d] %s %d\n", (int)gettid(), thread->name, ret);
    }
#else
	//ALOGI("SDL_RunThread: [%d] %s\n", (int)GetCurrentThreadId(), thread->name);
#endif

    thread->retval = thread->func(thread->data);
#ifdef __ANDROID__
    SDL_JNI_DetachThreadEnv(thread->name);
#endif
    return NULL;
}

SDL_Thread *SDL_CreateThreadEx(SDL_Thread *thread, int (*fn)(void *), void *data, const char *name)
{
    thread->func = fn;
    thread->data = data;
#ifdef _WIN32
	strncpy_s(thread->name, 32, name, sizeof(thread->name) - 1);
#else
	strlcpy(thread->name, name, sizeof(thread->name) - 1);
#endif // !_WIN32

    
    int retval = pthread_create(&thread->id, NULL, SDL_RunThread, thread);
    if (retval)
        return NULL;

    return thread;
}
#endif

int SDL_SetThreadPriority(SDL_ThreadPriority priority)
{
    struct sched_param sched;
    int policy;
    pthread_t thread = pthread_self();

    if (pthread_getschedparam(thread, &policy, &sched) < 0) {
        ALOGE("pthread_getschedparam() failed");
        return -1;
    }
    if (priority == SDL_THREAD_PRIORITY_LOW) {
        sched.sched_priority = sched_get_priority_min(policy);
    } else if (priority == SDL_THREAD_PRIORITY_HIGH) {
        sched.sched_priority = sched_get_priority_max(policy);
    } else {
        int min_priority = sched_get_priority_min(policy);
        int max_priority = sched_get_priority_max(policy);
        sched.sched_priority = (min_priority + (max_priority - min_priority) / 2);
    }
    if (pthread_setschedparam(thread, policy, &sched) < 0) {
        ALOGE("pthread_setschedparam() failed");
        return -1;
    }
    return 0;
}

void SDL_WaitThread(SDL_Thread *thread, int *status)
{
    if (!thread)
        return;

    pthread_join(thread->id, NULL);

    if (status)
        *status = thread->retval;
}

void SDL_DetachThread(SDL_Thread *thread)
{
    if (!thread)
        return;

    pthread_detach(thread->id);
}
