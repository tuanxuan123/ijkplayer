//
//  Shaders.metal
//  demo
//
//  Created by xanderdeng(邓轩颖) on 2019/5/6.
//  Copyright © 2019年 xanderdeng(邓轩颖). All rights reserved.
//

// File for Metal kernel and shader functions

#include <metal_stdlib>
#include <simd/simd.h>

// Including header shared between this Metal shader code and Swift/C code executing Metal API commands
#import "ShaderTypes.h"

using namespace metal;

typedef struct
{
    float3 position [[attribute(VertexAttributePosition)]];
    float2 texCoord [[attribute(VertexAttributeTexcoord)]];
} Vertex;

typedef struct
{
    float4 position [[position]];
    float2 texCoord;
} ColorInOut;

vertex ColorInOut vertexShader(Vertex in [[stage_in]],
                               constant Uniforms & uniforms [[ buffer(BufferIndexUniforms) ]])
{
    ColorInOut out;

    float4 position = float4(in.position, 1.0);
    out.position = uniforms.projectionMatrix * uniforms.modelViewMatrix * position;
    out.texCoord = in.texCoord;

    return out;
}

fragment float4 fragmentShader(ColorInOut in [[stage_in]],
                               constant Uniforms & uniforms [[ buffer(BufferIndexUniforms) ]],
                               texture2d<half> colorMap     [[ texture(TextureIndexColor) ]])
{
    constexpr sampler colorSampler(mip_filter::linear,
                                   mag_filter::linear,
                                   min_filter::linear);

    half4 colorSample   = colorMap.sample(colorSampler, in.texCoord.xy);

    return float4(colorSample);
}



vertex ColorInOut videoVertexShader(device Vertex *in [[buffer(0)]],  uint vid[[vertex_id]])
{
    ColorInOut out;
    float4 pos = float4(in[vid].position, 1.0);
    out.position = pos;
    out.texCoord = in[vid].texCoord;
    
    return out;
}


vertex ColorInOut videoVertexShader2(Vertex in [[stage_in]])
{
    ColorInOut out;
    
    float4 position = float4(in.position, 1.0);
    out.position = position;
    out.texCoord = in.texCoord;
    
    return out;
}


fragment float4 videoFragmentShader(ColorInOut in [[stage_in]],
                                    texture2d<half> videoMap [[ texture(TextureIndexColor) ]])
{
    constexpr sampler videoSampler(mip_filter::linear,
                                   mag_filter::linear,
                                   min_filter::linear);
    
    return float4(videoMap.sample(videoSampler, in.texCoord.xy));
}

