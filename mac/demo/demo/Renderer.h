//
//  Renderer.h
//  demo
//
//  Created by xanderdeng(邓轩颖) on 2019/4/10.
//  Copyright © 2019年 xanderdeng(邓轩颖). All rights reserved.
//

#import <MetalKit/MetalKit.h>

// Our platform independent renderer class.   Implements the MTKViewDelegate protocol which
//   allows it to accept per-frame update and drawable resize callbacks.
@interface Renderer : NSObject <MTKViewDelegate>

-(nonnull instancetype)initWithMetalKitView:(nonnull MTKView *)view;
-(void)updateVideoTexture:(int)index w:(int)w h:(int)h data:(unsigned char*)data;
@end

