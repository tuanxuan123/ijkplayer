/*
 * IJKSDLAudioQueueController.m
 *
 * Copyright (c) 2013-2014 Bilibili
 * Copyright (c) 2013-2014 Zhang Rui <bbcallen@gmail.com>
 *
 * based on https://github.com/kolyvan/kxmovie
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

#import "IJKSDLAudioQueueController.h"
#import "IJKSDLAudioKit.h"
#import "ijksdl_log.h"
#include "ff_ffplay.h"

#import <AVFoundation/AVFoundation.h>
#if !TARGET_OS_OSX
#import <UIKit/UIKit.h>
#endif


#define kIJKAudioQueueNumberBuffers (3)

@implementation IJKSDLAudioQueueController {
    AudioQueueRef _audioQueueRef;
    AudioQueueBufferRef _audioQueueBufferRefArray[kIJKAudioQueueNumberBuffers];
    BOOL _isPaused;
    BOOL _isStopped;

    volatile BOOL _isAborted;
    NSLock *_lock;
}

- (id)initWithAudioSpec:(const SDL_AudioSpec *)aSpec
{
    self = [super init];
    if (self) {
        if (aSpec == NULL) {
            self = nil;
            return nil;
        }
        _spec = *aSpec;

        if (aSpec->format != AUDIO_S16SYS) {
            NSLog(@"aout_open_audio: unsupported format %d\n", (int)aSpec->format);
            return nil;
        }



        /* Get the current format */
        AudioStreamBasicDescription streamDescription;
        IJKSDLGetAudioStreamBasicDescriptionFromSpec(&_spec, &streamDescription);

        SDL_CalculateAudioSpec(&_spec);

        if (_spec.size <= 0) {
            NSLog(@"aout_open_audio: unexcepted audio spec size %u", _spec.size);
            return nil;
        }

        OSStatus status = AudioQueueNewOutput(&streamDescription,
                                              IJKSDLAudioQueueOuptutCallback,
                                              (__bridge void *) self,
                                              NULL,
                                              kCFRunLoopCommonModes,
                                              0,
                                              &_audioQueueRef);
        if (status != noErr) {
            NSLog(@"AudioQueue: AudioQueueNewOutput failed (%d)\n", (int)status);
            self = nil;
            return nil;
        }

        UInt32 propValue = 1;
        AudioQueueSetProperty(_audioQueueRef, kAudioQueueProperty_EnableTimePitch, &propValue, sizeof(propValue));
        propValue = 1;
        AudioQueueSetProperty(_audioQueueRef, kAudioQueueProperty_TimePitchBypass, &propValue, sizeof(propValue));
        propValue = kAudioQueueTimePitchAlgorithm_Spectral;
        AudioQueueSetProperty(_audioQueueRef, kAudioQueueProperty_TimePitchAlgorithm, &propValue, sizeof(propValue));

        status = AudioQueueStart(_audioQueueRef, NULL);
        if (status != noErr) {
            NSLog(@"AudioQueue: AudioQueueStart failed (%d)\n", (int)status);
            self = nil;
            return nil;
        }

        for (int i = 0;i < kIJKAudioQueueNumberBuffers; i++)
        {
            AudioQueueAllocateBuffer(_audioQueueRef, _spec.size, &_audioQueueBufferRefArray[i]);
            _audioQueueBufferRefArray[i]->mAudioDataByteSize = _spec.size;
            memset(_audioQueueBufferRefArray[i]->mAudioData, 0, _spec.size);
            AudioQueueEnqueueBuffer(_audioQueueRef, _audioQueueBufferRefArray[i], 0, NULL);
        }
        
        _isStopped = NO;

        _lock = [[NSLock alloc] init];
    }
    return self;
}

- (void)dealloc
{
    [self close];
    [_lock release];
    _lock = nil;
    [super dealloc];
}

- (void)play
{
    if (!_audioQueueRef)
        return;

    @synchronized(_lock) {
        _isPaused = NO;
#if TARGET_OS_IOS
        NSError *error = nil;
        if (NO == [[AVAudioSession sharedInstance] setActive:YES error:&error]) {
            NSLog(@"AudioQueue: AVAudioSession.setActive(YES) failed: %@\n", error ? [error localizedDescription] : @"nil");
        }
#endif

        OSStatus status = AudioQueueStart(_audioQueueRef, NULL);
        if (status != noErr)
            NSLog(@"AudioQueue: AudioQueueStart failed (%d)\n", (int)status);
    }
}

- (void)pause
{
    if (!_audioQueueRef)
        return;

    @synchronized(_lock) {
        if (_isStopped)
            return;

        _isPaused = YES;
        OSStatus status = AudioQueuePause(_audioQueueRef);
        if (status != noErr)
            NSLog(@"AudioQueue: AudioQueuePause failed (%d)\n", (int)status);
    }
}

- (void)flush
{
    if (!_audioQueueRef)
        return;

    @synchronized(_lock) {
        if (_isStopped)
            return;

        if (_isPaused == YES) {
            for (int i = 0; i < kIJKAudioQueueNumberBuffers; i++)
            {
                if (_audioQueueBufferRefArray[i] && _audioQueueBufferRefArray[i]->mAudioData) {
                    _audioQueueBufferRefArray[i]->mAudioDataByteSize = _spec.size;
                    memset(_audioQueueBufferRefArray[i]->mAudioData, 0, _spec.size);
                }
            }
        } else {
            AudioQueueFlush(_audioQueueRef);
        }
    }
}

- (void)stop
{
    if (!_audioQueueRef)
        return;

    int asyncMode = 1;
    FFPlayer *ffp = _spec.userdata;
    
    if (ffp) {
        asyncMode = ffp->audio_stop_mode;
    }
#if !TARGET_OS_OSX
    float version = [[[UIDevice currentDevice] systemVersion] floatValue];
    
    if (version < 16.0){
        asyncMode = 0;
    }
#endif

    if (asyncMode) {
        @synchronized(_lock) {
            if (_isStopped)
                return;

            _isStopped = YES;
            AudioQueueStop(_audioQueueRef, false);
            AudioQueueDispose(_audioQueueRef, false);
            _audioQueueRef = nil;
            
        }
    } else {
        AudioQueueStop(_audioQueueRef, true);
        AudioQueueDispose(_audioQueueRef, true);
        _audioQueueRef = nil;
    }
}

- (void)close
{
    [self stop];
}

- (void)setPlaybackRate:(float)playbackRate
{
    if (fabsf(playbackRate - 1.0f) <= 0.000001) {
        UInt32 propValue = 1;
        AudioQueueSetProperty(_audioQueueRef, kAudioQueueProperty_TimePitchBypass, &propValue, sizeof(propValue));
        AudioQueueSetParameter(_audioQueueRef, kAudioQueueParam_PlayRate, 1.0f);
    } else {
        UInt32 propValue = 0;
        AudioQueueSetProperty(_audioQueueRef, kAudioQueueProperty_TimePitchBypass, &propValue, sizeof(propValue));
        AudioQueueSetParameter(_audioQueueRef, kAudioQueueParam_PlayRate, playbackRate);
    }
}

- (void)setPlaybackVolume:(float)playbackVolume
{
    float aq_volume = playbackVolume;
    if (fabsf(aq_volume - 1.0f) <= 0.000001) {
        AudioQueueSetParameter(_audioQueueRef, kAudioQueueParam_Volume, 1.f);
    } else {
        AudioQueueSetParameter(_audioQueueRef, kAudioQueueParam_Volume, aq_volume);
    }
}

- (double)get_latency_seconds
{
    return ((double)(kIJKAudioQueueNumberBuffers)) * _spec.samples / _spec.freq;
}


static void IJKSDLAudioQueueOuptutCallback(void * inUserData, AudioQueueRef inAQ, AudioQueueBufferRef inBuffer) {
    if (inAQ == nil || inBuffer == nil)
        return;

    @autoreleasepool {
        IJKSDLAudioQueueController* aqController = (__bridge IJKSDLAudioQueueController *) inUserData;

        if (aqController && !aqController->_isStopped) {
            @synchronized(aqController->_lock) {
                if (aqController->_isPaused || aqController->_isStopped) {
                    memset(inBuffer->mAudioData, aqController.spec.silence, inBuffer->mAudioDataByteSize);
                } else {
                    (*aqController.spec.callback)(aqController.spec.userdata, inBuffer->mAudioData, inBuffer->mAudioDataByteSize);
                    
                    FFPlayer *ffp = aqController.spec.userdata;
                    if (ffp != NULL) {
                        VideoState *is = ffp->is;
                        if (is != NULL) {
                            if (ffp->audio_delegate && is->audio_buf)
                            {
                                int nb_samples = inBuffer->mAudioDataByteSize / 2 / is->audio_src.channels;
                                ffp->audio_delegate(inBuffer->mAudioData, inBuffer->mAudioDataByteSize, nb_samples, is->audio_src.channels, is->audio_src.freq, is->audio_clock, ffp->tag);
                            }
                        }
                        if (!ffp->play_audio || ffp->only_play_api)
                        {
                            memset(inBuffer->mAudioData, aqController.spec.silence, inBuffer->mAudioDataByteSize);
                        }
                    }
                }
            }
        } else {
            memset(inBuffer->mAudioData, 0, inBuffer->mAudioDataByteSize);
        }

        AudioQueueEnqueueBuffer(inAQ, inBuffer, 0, NULL);
    }
}

@end
