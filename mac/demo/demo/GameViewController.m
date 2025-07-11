//
//  GameViewController.m
//  demo
//
//  Created by xanderdeng(邓轩颖) on 2019/4/10.
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
        [renderObj updateVideoTexture:w h:h data:data[tag] tag:tag];
    }
}

void MessageCallback(int event, int arg1, int arg2, const char* msg, int tag)
{
    printf("MessageCallback event:%d, msg:%s\n", event, msg);
}

@implementation GameViewController
{
    MTKView *_view;

    Renderer *_renderer;
}

- (void)viewDidLoad
{
    [super viewDidLoad];
    _view = (MTKView *)self.view;
    
    _view.device = MTLCreateSystemDefaultDevice();

    if(!_view.device)
    {
        NSLog(@"Metal is not supported on this device");
        self.view = [[NSView alloc] initWithFrame:self.view.frame];
        return;
    }
    _renderer = [[Renderer alloc] initWithMetalKitView:_view];

    [_renderer mtkView:_view drawableSizeWillChange:_view.bounds.size];

    _view.delegate = _renderer;
    renderObj = _renderer;
    h5_video_init(UpdateTextureData, MessageCallback, FMT_RGBA);
    pandora_video_init(PandoraUpdateTextureData, MessageCallback);
    
    
    static int playing_video_cnt = 0;
    // h5_video_set_cache_path("/Users/kiddpeng/Workspace/test"); // 用于测试缓存功能
    // pandora_video_set_cache_path("/Users/kiddpeng/Workspace/test"); // 用于测试缓存功能
    id monitor = [NSEvent addLocalMonitorForEventsMatchingMask:NSEventMaskKeyDown handler:^NSEvent * _Nullable(NSEvent * _Nonnull aEvent) {
        if(aEvent.keyCode == 0x31) { //space
            if (playing_video_cnt < 2)
            {
                if (playing_video_cnt == 0)
                {
                    const char *url = "ijkio:cache:ffio:https://image.smoba.qq.com/Video/playonline/Nobe_Video.mp4";
                    const char *save_url = "cache:ffio:https://image.smoba.qq.com/Video/playonline/Nobe_Video.mp4";
                    int ret = pandora_video_is_cache_complete(save_url);
                    printf("pandora_video_is_cache_complete=%d\n", ret);
                    if (ret == 1)
                    {
                        char *savePath = pandora_video_cache_save_to_mp4(save_url);
                        printf("pandora_video_cache_save_to_mp4 savePath=%s\n", savePath);
                    }
                    int tag = h5_video_play(url, true, 0);
                    [renderObj setVideoTag1:tag];
                }
                else
                {
                    pandora_video_set_cache_path("/Users/kiddpeng/Workspace/test");
                    int tag = pandora_video_play("ijkio:cache:ffio:https://image.smoba.qq.com/Video/playonline/Nobe_Video.mp4", true);
                    [renderObj setVideoTag2:tag];
                }

                playing_video_cnt++;
            }

        } else if(aEvent.keyCode == 0x30) { //tab
            if (playing_video_cnt > 0)
            {
                
                if (playing_video_cnt == 1)
                {
                    h5_video_stop(playing_video_cnt - 1);
                }
                else
                {
                    pandora_video_stop(playing_video_cnt - 1);
                }
                playing_video_cnt--;
            }

        }
        
            return aEvent;
        }];
    
    /*tag = h5_video_play("http://apd-6ac1f11b6ac7d9e8ba92cf66ca8d673d.v.smtcdns.com/vpandora.tc.qq.com/1093_415DBB63B2E3722D6DE37FD85BEE90F0.f0.x-flv?vkey=DA90FC5844D72EC5AEBC003D14E9F864AD9BA19A9F05AB851F81693C3E55DF6B2ED570C6029F795051F79C7AE51BDFF6DD77BA2DA0B53934D46B419DD8B2103D55F1C831DC6349BEF170FD502C1C02EBA035F3BDEEEAF3BF", true, 0);
    [renderObj setVideoTag2:tag];*/
}

@end
