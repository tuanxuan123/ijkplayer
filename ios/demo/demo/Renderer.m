//
//  Renderer.m
//  demo
//
//  Created by xanderdeng(邓轩颖) on 2019/5/6.
//  Copyright © 2019年 xanderdeng(邓轩颖). All rights reserved.
//

#import <simd/simd.h>
#import <ModelIO/ModelIO.h>

#import "Renderer.h"

// Include header shared between C code here, which executes Metal API commands, and .metal files
#import "ShaderTypes.h"
#include "h5_player.h"
#include "pandora_player.h"

typedef struct
{
    vector_float3 position;
    vector_float2 texCoord;
} PlaneVertex;

static const NSUInteger kMaxBuffersInFlight = 3;



@implementation Renderer
{
    dispatch_semaphore_t _inFlightSemaphore;
    id <MTLDevice> _device;
    id <MTLCommandQueue> _commandQueue;
    
    id <MTLRenderPipelineState> _pipelineState;
    id <MTLDepthStencilState> _depthState;
    id <MTLTexture> _colorMap;
    id <MTLTexture> _videoTextures[2];
    id <MTLBuffer> _vertexBuffers[2];
    id <MTLRenderPipelineState> _pipelineVideo;
    MTLVertexDescriptor *_mtlVertexDescriptor;
    
}

-(nonnull instancetype)initWithMetalKitView:(nonnull MTKView *)view;
{
    self = [super init];
    if(self)
    {
        _device = view.device;
        _inFlightSemaphore = dispatch_semaphore_create(kMaxBuffersInFlight);
        [self _loadMetalWithView:view];
        [self _loadAssets];
    }
    
    return self;
}

-(void)updateVideoTexture:(int)w h:(int)h data:(unsigned char*)data tag:(int)tag;
{
    int index = 0;
    if(tag == _VideoTag1)
        index = 0;
    else if(tag == _VideoTag2)
        index = 1;
    else
        return;
    
    if (index == 0)
    {
        if(_videoTextures[index] == nil || _videoTextures[index].width != w || _videoTextures[index].height != h)
        {
            MTLTextureDescriptor* textureDesc = [MTLTextureDescriptor texture2DDescriptorWithPixelFormat:MTLPixelFormatRGBA8Unorm_sRGB width:w height:h mipmapped:NO];
            _videoTextures[index] = [_device newTextureWithDescriptor:textureDesc];
        
        }
    
        MTLRegion region = MTLRegionMake2D(0, 0, w, h);
        [_videoTextures[index] replaceRegion:region mipmapLevel:0 withBytes:data bytesPerRow:4*w];
    }
    else
    {
        void* ref = (void*)data;
        CVMetalTextureRef y_texture_ref = (CVMetalTextureRef)ref;
        _videoTextures[index] = CVMetalTextureGetTexture(y_texture_ref);
    }
    
    NSLog(@"updateVideoTexture index: %d", index);
    
}

- (void)_loadMetalWithView:(nonnull MTKView *)view;
{
    /// Load Metal state objects and initalize renderer dependent view properties
    
    view.depthStencilPixelFormat = MTLPixelFormatDepth32Float_Stencil8;
    view.colorPixelFormat = MTLPixelFormatBGRA8Unorm_sRGB;
    view.sampleCount = 1;
    
    _mtlVertexDescriptor = [[MTLVertexDescriptor alloc] init];
    
    _mtlVertexDescriptor.attributes[VertexAttributePosition].format = MTLVertexFormatFloat3;
    _mtlVertexDescriptor.attributes[VertexAttributePosition].offset = 0;
    _mtlVertexDescriptor.attributes[VertexAttributePosition].bufferIndex = BufferIndexMeshPositions;
    
    _mtlVertexDescriptor.attributes[VertexAttributeTexcoord].format = MTLVertexFormatFloat2;
    _mtlVertexDescriptor.attributes[VertexAttributeTexcoord].offset = 0;
    _mtlVertexDescriptor.attributes[VertexAttributeTexcoord].bufferIndex = BufferIndexMeshGenerics;
    
    _mtlVertexDescriptor.layouts[BufferIndexMeshPositions].stride = 12;
    _mtlVertexDescriptor.layouts[BufferIndexMeshPositions].stepRate = 1;
    _mtlVertexDescriptor.layouts[BufferIndexMeshPositions].stepFunction = MTLVertexStepFunctionPerVertex;
    
    _mtlVertexDescriptor.layouts[BufferIndexMeshGenerics].stride = 8;
    _mtlVertexDescriptor.layouts[BufferIndexMeshGenerics].stepRate = 1;
    _mtlVertexDescriptor.layouts[BufferIndexMeshGenerics].stepFunction = MTLVertexStepFunctionPerVertex;
    
    id<MTLLibrary> defaultLibrary = [_device newDefaultLibrary];
    
    id <MTLFunction> vertexFunction = [defaultLibrary newFunctionWithName:@"vertexShader"];
    
    id <MTLFunction> fragmentFunction = [defaultLibrary newFunctionWithName:@"fragmentShader"];
    
    MTLRenderPipelineDescriptor *pipelineStateDescriptor = [[MTLRenderPipelineDescriptor alloc] init];
    pipelineStateDescriptor.label = @"MyPipeline";
    pipelineStateDescriptor.sampleCount = view.sampleCount;
    pipelineStateDescriptor.vertexFunction = vertexFunction;
    pipelineStateDescriptor.fragmentFunction = fragmentFunction;
    pipelineStateDescriptor.vertexDescriptor = _mtlVertexDescriptor;
    pipelineStateDescriptor.colorAttachments[0].pixelFormat = view.colorPixelFormat;
    pipelineStateDescriptor.depthAttachmentPixelFormat = view.depthStencilPixelFormat;
    pipelineStateDescriptor.stencilAttachmentPixelFormat = view.depthStencilPixelFormat;
    
    NSError *error = NULL;
    _pipelineState = [_device newRenderPipelineStateWithDescriptor:pipelineStateDescriptor error:&error];
    if (!_pipelineState)
    {
        NSLog(@"Failed to created pipeline state, error %@", error);
    }
    
    [self createVideoPipeline:defaultLibrary view:view];
    
    MTLDepthStencilDescriptor *depthStateDesc = [[MTLDepthStencilDescriptor alloc] init];
    depthStateDesc.depthCompareFunction = MTLCompareFunctionLess;
    depthStateDesc.depthWriteEnabled = YES;
    _depthState = [_device newDepthStencilStateWithDescriptor:depthStateDesc];
    
    
    _commandQueue = [_device newCommandQueue];
}

- (void)createVideoPipeline:(id<MTLLibrary>)library view:(nonnull MTKView *)view;
{
    MTLVertexDescriptor *vertexDescriptor = [[MTLVertexDescriptor alloc] init];
    vertexDescriptor.attributes[VertexAttributePosition].format = MTLVertexFormatFloat3;
    vertexDescriptor.attributes[VertexAttributePosition].offset = 0;
    vertexDescriptor.attributes[VertexAttributePosition].bufferIndex = 0;
    
    vertexDescriptor.attributes[VertexAttributeTexcoord].format = MTLVertexFormatFloat2;
    vertexDescriptor.attributes[VertexAttributeTexcoord].offset = 0;
    vertexDescriptor.attributes[VertexAttributeTexcoord].bufferIndex = 1;
    
    vertexDescriptor.layouts[VertexAttributePosition].stride = sizeof(vector_float3);
    vertexDescriptor.layouts[VertexAttributePosition].stepRate = 1;
    vertexDescriptor.layouts[VertexAttributePosition].stepFunction = MTLVertexStepFunctionPerVertex;
    
    vertexDescriptor.layouts[VertexAttributeTexcoord].stride = sizeof(vector_float2);
    vertexDescriptor.layouts[VertexAttributeTexcoord].stepRate = 1;
    vertexDescriptor.layouts[VertexAttributeTexcoord].stepFunction = MTLVertexStepFunctionPerVertex;
    
    //id <MTLFunction> vertexFunction = [library newFunctionWithName:@"videoVertexShader2"];
    id <MTLFunction> vertexFunction = [library newFunctionWithName:@"videoVertexShader"];
    id <MTLFunction> fragmentFunction = [library newFunctionWithName:@"videoFragmentShader"];
    
    MTLRenderPipelineDescriptor *pipelineDescriptor = [[MTLRenderPipelineDescriptor alloc] init];
    pipelineDescriptor.label = @"videoPipeline";
    //pipelineDescriptor.vertexDescriptor = vertexDescriptor;
    pipelineDescriptor.vertexFunction = vertexFunction;
    pipelineDescriptor.fragmentFunction = fragmentFunction;
    pipelineDescriptor.colorAttachments[0].pixelFormat = view.colorPixelFormat;
    pipelineDescriptor.depthAttachmentPixelFormat = view.depthStencilPixelFormat;
    pipelineDescriptor.stencilAttachmentPixelFormat = view.depthStencilPixelFormat;
    
    NSError *error = NULL;
    _pipelineVideo = [_device newRenderPipelineStateWithDescriptor:pipelineDescriptor error:&error];
    
    if(!_pipelineVideo)
    {
        NSLog(@"Failed to created pipeline state, error %@", error);
    }
}

- (void)makeBuffer
{
    static const PlaneVertex vertices1[] =
    {
        { .position = {  -0.95,  0.35, 0.0}, .texCoord = { 0.0, 0.0} },
        { .position = { -0.95, -0.35, 0.0}, .texCoord = { 0.0, 1.0} },
        { .position = {  -0.05,  0.35, 0.0}, .texCoord = { 1.0, 0.0} },
        
        { .position = { -0.05,  0.35, 0.0}, .texCoord = { 1.0, 0.0} },
        { .position = { -0.95, -0.35, 0.0}, .texCoord = { 0.0, 1.0} },
        { .position = { -0.05, -0.35, 0.0}, .texCoord = { 1.0, 1.0} },
    };
    
    if(_vertexBuffers[0] == nil)
        _vertexBuffers[0] = [_device newBufferWithBytes:vertices1 length:sizeof(vertices1) options:MTLResourceOptionCPUCacheModeDefault];
    
    static const PlaneVertex vertices2[] =
    {
        { .position = {  0.05,  0.35, 0.0}, .texCoord = { 0.0, 0.0} },
        { .position = {  0.05, -0.35, 0.0}, .texCoord = { 0.0, 1.0} },
        { .position = {  0.95,  0.35, 0.0}, .texCoord = { 1.0, 0.0} },
        
        { .position = {  0.95,  0.35, 0.0}, .texCoord = { 1.0, 0.0} },
        { .position = {  0.05, -0.35, 0.0}, .texCoord = { 0.0, 1.0} },
        { .position = {  0.95, -0.35, 0.0}, .texCoord = { 1.0, 1.0} },
    };
    
    if(_vertexBuffers[1] == nil)
        _vertexBuffers[1] = [_device newBufferWithBytes:vertices2 length:sizeof(vertices2) options:MTLResourceOptionCPUCacheModeDefault];
}

- (void)_loadAssets
{
    /// Load assets into metal objects
    NSError *error;
    
    MTKTextureLoader* textureLoader = [[MTKTextureLoader alloc] initWithDevice:_device];
    
    NSDictionary *textureLoaderOptions =
    @{
      MTKTextureLoaderOptionTextureUsage       : @(MTLTextureUsageShaderRead),
      MTKTextureLoaderOptionTextureStorageMode : @(MTLStorageModePrivate)
      };
    
    _colorMap = [textureLoader newTextureWithName:@"ColorMap"
                                      scaleFactor:1.0
                                           bundle:nil
                                          options:textureLoaderOptions
                                            error:&error];
    
    if(!_colorMap || error)
    {
        NSLog(@"Error creating texture %@", error.localizedDescription);
    }
    
    [self makeBuffer];
    
}




- (void)drawInMTKView:(nonnull MTKView *)view
{
    /// Per frame updates here
    //h5_video_update(_VideoTag1);
    //h5_video_update(_VideoTag2);
    
    for(int i = 0; i < 10; ++i)
        h5_video_update(i);
 
    for(int i = 0; i < 10; ++i)
        pandora_video_update(i);
    
    dispatch_semaphore_wait(_inFlightSemaphore, DISPATCH_TIME_FOREVER);
    
    id <MTLCommandBuffer> commandBuffer = [_commandQueue commandBuffer];
    commandBuffer.label = @"MyCommand";
    
    __block dispatch_semaphore_t block_sema = _inFlightSemaphore;
    [commandBuffer addCompletedHandler:^(id<MTLCommandBuffer> buffer)
     {
         dispatch_semaphore_signal(block_sema);
     }];
    

    
    /// Delay getting the currentRenderPassDescriptor until we absolutely need it to avoid
    ///   holding onto the drawable and blocking the display pipeline any longer than necessary
    MTLRenderPassDescriptor* renderPassDescriptor = view.currentRenderPassDescriptor;
    
    if(renderPassDescriptor != nil) {
        /// Final pass rendering code here
        
        id <MTLRenderCommandEncoder> renderEncoder =
        [commandBuffer renderCommandEncoderWithDescriptor:renderPassDescriptor];
        renderEncoder.label = @"VideoRenderEncoder";
        
        [renderEncoder pushDebugGroup:@"DrawVideo"];
        [renderEncoder setCullMode:MTLCullModeBack];
        [renderEncoder setRenderPipelineState:_pipelineState];
        [renderEncoder setDepthStencilState:_depthState];
        
        [self drawVideo:renderEncoder];
        [renderEncoder popDebugGroup];
        
        [renderEncoder endEncoding];
        
        
        
        [commandBuffer presentDrawable:view.currentDrawable];
    }
    
    [commandBuffer commit];
}

- (void)drawVideo:(id <MTLRenderCommandEncoder>)encoder
{
    
    [encoder setFrontFacingWinding:MTLWindingCounterClockwise];
    [encoder setCullMode:MTLCullModeBack];
    [encoder setRenderPipelineState:_pipelineVideo];
    [encoder setDepthStencilState:_depthState];
    
    for(int i = 0; i < 2; ++i)
    {
        [encoder setVertexBuffer:_vertexBuffers[i] offset:0 atIndex:0];
        if(_videoTextures[i])
        {
            [encoder setFragmentTexture:_videoTextures[i]
                                atIndex:TextureIndexColor];
        }
        else
        {
            [encoder setFragmentTexture:_colorMap
                                atIndex:TextureIndexColor];
        }
        
        [encoder drawPrimitives:MTLPrimitiveTypeTriangle vertexStart:0 vertexCount:6];
    }
    
}

- (void)mtkView:(nonnull MTKView *)view drawableSizeWillChange:(CGSize)size
{

}

#pragma mark Matrix Math Utilities

matrix_float4x4 matrix4x4_translation(float tx, float ty, float tz)
{
    return (matrix_float4x4) {{
        { 1,   0,  0,  0 },
        { 0,   1,  0,  0 },
        { 0,   0,  1,  0 },
        { tx, ty, tz,  1 }
    }};
}

static matrix_float4x4 matrix4x4_rotation(float radians, vector_float3 axis)
{
    axis = vector_normalize(axis);
    float ct = cosf(radians);
    float st = sinf(radians);
    float ci = 1 - ct;
    float x = axis.x, y = axis.y, z = axis.z;
    
    return (matrix_float4x4) {{
        { ct + x * x * ci,     y * x * ci + z * st, z * x * ci - y * st, 0},
        { x * y * ci - z * st,     ct + y * y * ci, z * y * ci + x * st, 0},
        { x * z * ci + y * st, y * z * ci - x * st,     ct + z * z * ci, 0},
        {                   0,                   0,                   0, 1}
    }};
}

matrix_float4x4 matrix_perspective_right_hand(float fovyRadians, float aspect, float nearZ, float farZ)
{
    float ys = 1 / tanf(fovyRadians * 0.5);
    float xs = ys / aspect;
    float zs = farZ / (nearZ - farZ);
    
    return (matrix_float4x4) {{
        { xs,   0,          0,  0 },
        {  0,  ys,          0,  0 },
        {  0,   0,         zs, -1 },
        {  0,   0, nearZ * zs,  0 }
    }};
}

@end

