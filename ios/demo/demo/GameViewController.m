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

Renderer *renderObj = NULL;

void UpdateTextureData(int index, int w, int h, unsigned char *data)
{
    if(renderObj)
    {
        [renderObj updateVideoTexture:0 w:w h:h data:data];
    }
}


void MessageCallback(int index, int event, int arg1, int arg2, const char* msg)
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
    h5_video_init(UpdateTextureData, MessageCallback);
    h5_video_play("http://1253131631.vod2.myqcloud.com/26f327f9vodgzp1253131631/bca0bd469031868222923928043/f0.mp4", 0, 0);
}

@end
