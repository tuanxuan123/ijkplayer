//
//  GameViewController.m
//  demo
//
//  Created by xanderdeng(邓轩颖) on 2019/5/6.
//  Copyright © 2019年 xanderdeng(邓轩颖). All rights reserved.
//

#import "GameViewController.h"
#import "Renderer.h"
#include "h5_player.h"
#include "pandora_player.h"

Renderer *renderObj = NULL;

void UpdateTextureData(int w, int h, unsigned char *data, int tag)
{
    if(renderObj)
    {
        [renderObj updateVideoTexture:w h:h data:data tag:tag];
    }
}

void PandoraUpdateTextureData(uint8_t* data[], int w, int h, int format, int tag)
{
    if(renderObj)
    {
        [renderObj updateVideoTexture:w h:h data:data[0] tag:tag];
    }
}

void MessageCallback(int event, int arg1, int arg2, const char* msg, int tag)
{
    printf("MessageCallback event:%d, arg1:%d, arg2:%d, msg:%s\n", event, arg1, arg2, msg);
    
    if(event == 200)
        h5_video_set_volume(50, 0);
}

@implementation GameViewController
{
    MTKView *_view;

    Renderer *_renderer;
    bool _isPlaying;
    bool _isPlaying2;
    int _tag;
    int _tag2;
}

- (void)viewDidLoad
{
    [super viewDidLoad];

    _view = (MTKView *)self.view;

    _view.device = MTLCreateSystemDefaultDevice();
    _view.backgroundColor = UIColor.blackColor;

    if(!_view.device)
    {
        NSLog(@"Metal is not supported on this device");
        self.view = [[UIView alloc] initWithFrame:self.view.frame];
        return;
    }

    _renderer = [[Renderer alloc] initWithMetalKitView:_view];

    [_renderer mtkView:_view drawableSizeWillChange:_view.bounds.size];

    _view.delegate = _renderer;
    
    renderObj = _renderer;
    h5_video_init(UpdateTextureData, MessageCallback, FMT_RGBA);
	
	pandora_video_init(PandoraUpdateTextureData, MessageCallback);
    
    pandora_video_set_render_device((__bridge void*)_view.device);
    pandora_video_use_surface(1);
    
    [[NSNotificationCenter defaultCenter] addObserver:self
                                                 selector:@selector(foregroundNotification:)
                                                     name:UIApplicationWillEnterForegroundNotification
                                                   object:nil];
    
    [[NSNotificationCenter defaultCenter] addObserver:self
                                                 selector:@selector(backgroundNotification:)
                                                     name:UIApplicationDidEnterBackgroundNotification
                                                   object:nil];
    
    /*NSString *videoPath = [[NSBundle mainBundle] pathForResource:@"header" ofType:@"mp4"];
    NSString *videoPath1 = [[NSBundle mainBundle] pathForResource:@"LoginVideo" ofType:@"mp4"];
    
    int tag = h5_video_play([videoPath UTF8String], true);//("http://1253131631.vod2.myqcloud.com/26f327f9vodgzp1253131631/bca0bd469031868222923928043/f0.mp4", false);
    [renderObj setVideoTag1:tag];
    
    tag = h5_video_play([videoPath UTF8String], true);
    [renderObj setVideoTag2:tag];
    
    tag = h5_video_play([videoPath UTF8String], true);

    h5_video_play([videoPath UTF8String], true);
    h5_video_play([videoPath UTF8String], true);
    h5_video_play([videoPath UTF8String], true);*/
}


- (void)foregroundNotification:(NSNotification *)noti
{
    NSLog(@"===foregroundNotification==\n");
    
    if (_isPlaying)
    {
        h5_video_resume(_tag);
    }
    
    if (_isPlaying2)
    {
        pandora_video_resume(_tag2);
    }
}


- (void)backgroundNotification:(NSNotification *)noti
{
    NSLog(@"===backgroundNotification==\n");
    
    if (_isPlaying)
    {
        h5_video_pause(_tag);
    }
    
    if (_isPlaying2)
    {
        pandora_video_pause(_tag2);
    }
}


- (void)touchesEnded:(NSSet<UITouch *> *)touches withEvent:(nullable UIEvent *)event
{
    if(_isPlaying)
    {
        _isPlaying = false;
        for(int i = 0; i < 10; ++i)
            h5_video_stop(i);
    }
    else
    {
        _isPlaying = true;
        NSString *videoPath = [[NSBundle mainBundle] pathForResource:@"LoginVideo" ofType:@"mp4"];
        _tag = h5_video_play([videoPath UTF8String], 0,0);
        //int tag = h5_video_play("http://1251779387.vod2.myqcloud.com/0a3d4814vodcq1251779387/3e795ff33270835011105658520/ApWwbBLiaboA.mp4", 0, 0);//("https://image.smoba.qq.com/Video/playonline/Nobe_Video.mp4", 0, 0);
        [renderObj setVideoTag1:_tag];
        
        h5_video_set_volume(10, _tag);
        
        //tag = h5_video_play([videoPath UTF8String], true);
        //[renderObj setVideoTag2:tag];
    }
    
    if(_isPlaying2)
    {
        _isPlaying2 = false;
        for(int i = 0; i < 10; ++i)
            pandora_video_stop(i);
    }
    else
    {
        _isPlaying2 = true;
        NSString *videoPath = [[NSBundle mainBundle] pathForResource:@"LoginVideo" ofType:@"mp4"];
        _tag2 = pandora_video_play("https://image.smoba.qq.com/Video/playonline/Nobe_Video.mp4", 0);
        //int tag = h5_video_play("http://1251779387.vod2.myqcloud.com/0a3d4814vodcq1251779387/3e795ff33270835011105658520/ApWwbBLiaboA.mp4", 0, 0);//("https://image.smoba.qq.com/Video/playonline/Nobe_Video.mp4", 0, 0);
        [renderObj setVideoTag2:_tag2];
        
        //pandora_video_set_volume(20, _tag2);
    }
}

@end
