//========================================================================
// Simple GLFW+Metal example
// Copyright (c) Camilla Berglund <elmindreda@elmindreda.org>
//
// This software is provided 'as-is', without any express or implied
// warranty. In no event will the authors be held liable for any damages
// arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented; you must not
//    claim that you wrote the original software. If you use this software
//    in a product, an acknowledgment in the product documentation would
//    be appreciated but is not required.
//
// 2. Altered source versions must be plainly marked as such, and must not
//    be misrepresented as being the original software.
//
// 3. This notice may not be removed or altered from any source
//    distribution.
//
//========================================================================
//! [code]

#define GLFW_INCLUDE_NONE
#import <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_COCOA
#import <GLFW/glfw3native.h>

#import <Metal/Metal.h>
#import <QuartzCore/QuartzCore.h>
#import <simd/simd.h>

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>

static void error_callback(int error, const char* description)
{
    fputs(description, stderr);
}

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GLFW_TRUE);
}

int main(void)
{
	id<MTLDevice> device = MTLCreateSystemDefaultDevice();
	if (!device)
		exit(EXIT_FAILURE);

    glfwSetErrorCallback(error_callback);

    if (!glfwInit())
        exit(EXIT_FAILURE);

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    GLFWwindow* window = glfwCreateWindow(640, 480, "Metal Example", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    NSWindow* nswin = glfwGetCocoaWindow(window);
    CAMetalLayer* layer = [CAMetalLayer layer];
    layer.device = device;
    layer.pixelFormat = MTLPixelFormatBGRA8Unorm;
    nswin.contentView.layer = layer;
    nswin.contentView.wantsLayer = YES;

    MTLCompileOptions* compileOptions = [MTLCompileOptions new];
    compileOptions.languageVersion = MTLLanguageVersion1_1;
    NSError* compileError;
    id<MTLLibrary> lib = [device newLibraryWithSource:
       @"#include <metal_stdlib>\n"
        "using namespace metal;\n"
        "vertex float4 v_simple(\n"
        "    constant float4* in  [[buffer(0)]],\n"
        "    uint             vid [[vertex_id]])\n"
        "{\n"
        "    return in[vid];\n"
        "}\n"
        "fragment float4 f_simple(\n"
        "    float4 in [[stage_in]])\n"
        "{\n"
        "    return float4(1, 0, 0, 1);\n"
        "}\n"
       options:compileOptions error:&compileError];
    if (!lib)
    {
        NSLog(@"can't create library: %@", compileError);
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    id<MTLFunction> vs = [lib newFunctionWithName:@"v_simple"];
    assert(vs);
    id<MTLFunction> fs = [lib newFunctionWithName:@"f_simple"];
    assert(fs);

    id<MTLCommandQueue> cq = [device newCommandQueue];
    assert(cq);

    MTLRenderPipelineDescriptor* rpd = [MTLRenderPipelineDescriptor new];
    rpd.vertexFunction = vs;
    rpd.fragmentFunction = fs;
    rpd.colorAttachments[0].pixelFormat = layer.pixelFormat;
    id<MTLRenderPipelineState> rps = [device newRenderPipelineStateWithDescriptor:rpd error:NULL];
    assert(rps);

    glfwSetKeyCallback(window, key_callback);

    while (!glfwWindowShouldClose(window))
    {
        float ratio;
        int width, height;

        glfwGetFramebufferSize(window, &width, &height);
        ratio = width / (float) height;

        layer.drawableSize = CGSizeMake(width, height);
        id<CAMetalDrawable> drawable = [layer nextDrawable];
        assert(drawable);

        id<MTLCommandBuffer> cb = [cq commandBuffer];

        MTLRenderPassDescriptor* rpd = [MTLRenderPassDescriptor new];
        MTLRenderPassColorAttachmentDescriptor* cd = rpd.colorAttachments[0];
        cd.texture = drawable.texture;
        cd.loadAction = MTLLoadActionClear;
        cd.clearColor = MTLClearColorMake(1.0, 1.0, 1.0, 1.0);
        cd.storeAction = MTLStoreActionStore;
        id<MTLRenderCommandEncoder> rce = [cb renderCommandEncoderWithDescriptor:rpd];

        [rce setRenderPipelineState:rps];
        [rce setVertexBytes:(vector_float4[]){
            { 0, 0, 0, 1 },
            { -1, 1, 0, 1 },
            { 1, 1, 0, 1 },
        } length:3 * sizeof(vector_float4) atIndex:0];
        [rce drawPrimitives:MTLPrimitiveTypeTriangle vertexStart:0 vertexCount:3];

        [rce endEncoding];
        [cb presentDrawable:drawable];
        [cb commit];

        glfwPollEvents();
    }

    glfwDestroyWindow(window);

    glfwTerminate();
    exit(EXIT_SUCCESS);
}

//! [code]
