//========================================================================
// WebGPU triangle example
// Copyright (c) Sebastian Dawid <sebdawid@gmail.com>
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

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>

#include <webgpu/webgpu.h>
#include <webgpu/wgpu.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#define SHADER_SOURCE(...) #__VA_ARGS__

static const char* CODE = SHADER_SOURCE(
@vertex
fn vert(@builtin(vertex_index) index: u32) -> @builtin(position) vec4f {
    var p = vec2f(0.0, 0.0);
    
    if (index == 0u) {
        p = vec2f(-0.5, -0.5);
    } else if (index == 1u) {
        p = vec2f(0.5, -0.5);
    } else {
        p = vec2f(0.0, 0.5);
    }

    return vec4f(p, 0.0, 1.0);
}

@fragment
fn frag() -> @location(0) vec4f {
    return vec4f(1.0, 0.0, 0.0, 1.0);
}
);

static void error_callback(int error, const char* description);
static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);

static WGPULimits get_required_limits(WGPULimits* supported_limits);
static void adapter_callback(WGPURequestAdapterStatus status, WGPUAdapter adapter, WGPUStringView message, void* userdata1, void* userdata2);
static void device_callback(WGPURequestDeviceStatus status, WGPUDevice device, WGPUStringView message, void* userdata1, void* userdata2);
static void device_lost_callback(const WGPUDevice* device, WGPUDeviceLostReason reason, WGPUStringView message, void* userdata1, void* userdata2);
static void uncaptured_error_callback(const WGPUDevice* device, WGPUErrorType error, WGPUStringView message, void* userdata1, void* userdata2);

int main(void)
{
    glfwSetErrorCallback(error_callback);
    if (!glfwInit())
        exit(EXIT_FAILURE);
    glfwSetWGPUInstanceCreateSurfaceAddr(wgpuInstanceCreateSurface);

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    GLFWwindow* window = glfwCreateWindow(800, 600, "WebGPU Triangle", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    glfwSetKeyCallback(window, key_callback);

    WGPUInstance instance = wgpuCreateInstance(NULL);
    if (!instance)
    {
        fprintf(stderr, "Error: Failed to create WebGPU instance.\n");
        glfwDestroyWindow(window);
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    WGPUSurface surface = glfwCreateWindowWGPUSurface(instance, window);
    if (!surface)
    {
        fprintf(stderr, "Error: Failed to create WebGPU surface.\n");
        wgpuInstanceRelease(instance);
        glfwDestroyWindow(window);
        glfwTerminate();
        exit(EXIT_FAILURE);
    }
    WGPUAdapter adapter;
    WGPUDevice device;
    WGPUQueue queue;
    bool success = false;

    WGPURequestAdapterOptions adapter_options = {0};
    adapter_options.nextInChain = NULL;
    adapter_options.compatibleSurface = surface;

    WGPURequestAdapterCallbackInfo adapter_callback_info = {0};
    adapter_callback_info.nextInChain = NULL;
    adapter_callback_info.mode = WGPUCallbackMode_AllowSpontaneous;
    adapter_callback_info.callback = adapter_callback;
    adapter_callback_info.userdata1 = &adapter;
    adapter_callback_info.userdata2 = &success;

    wgpuInstanceRequestAdapter(instance, &adapter_options, adapter_callback_info);

    while (!success); // spin until we get the adapter
    success = false;

    WGPULimits supported_limits = {0};
    wgpuAdapterGetLimits(adapter, &supported_limits);
    WGPULimits required_limits = get_required_limits(&supported_limits);

    WGPUDeviceDescriptor device_descriptor = {0};
    device_descriptor.nextInChain = NULL;
    device_descriptor.label = (WGPUStringView) { "device", 6 };
    device_descriptor.requiredFeatureCount = 0;
    device_descriptor.requiredFeatures = NULL;
    device_descriptor.requiredLimits = &required_limits;

    device_descriptor.defaultQueue.nextInChain = NULL;
    device_descriptor.defaultQueue.label = (WGPUStringView) { "queue", 5 };

    device_descriptor.deviceLostCallbackInfo.nextInChain = NULL;
    device_descriptor.deviceLostCallbackInfo.mode = WGPUCallbackMode_AllowSpontaneous;
    device_descriptor.deviceLostCallbackInfo.callback = device_lost_callback;
    device_descriptor.deviceLostCallbackInfo.userdata1 = NULL;
    device_descriptor.deviceLostCallbackInfo.userdata2 = NULL;

    WGPURequestDeviceCallbackInfo device_callback_info = {0};
    device_callback_info.nextInChain = NULL;
    device_callback_info.mode = WGPUCallbackMode_AllowSpontaneous;
    device_callback_info.callback = device_callback;
    device_callback_info.userdata1 = &device;
    device_callback_info.userdata2 = &success;

    wgpuAdapterRequestDevice(adapter, &device_descriptor, device_callback_info);

    while (!success); // spin until we get the dvice
    success = false;

    queue = wgpuDeviceGetQueue(device);

    WGPUSurfaceCapabilities surface_capabilities = {0};
    surface_capabilities.nextInChain = NULL;
    if (wgpuSurfaceGetCapabilities(surface, adapter, &surface_capabilities) != WGPUStatus_Success)
    {
        fprintf(stderr, "Error: Failed to get surface capabilities.\n");
        wgpuQueueRelease(queue);
        wgpuDeviceRelease(device);
        wgpuSurfaceRelease(surface);
        glfwDestroyWindow(window);
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    WGPUTextureFormat format = {0};
    for (size_t i = 0; i < surface_capabilities.formatCount; ++i)
    {
        WGPUTextureFormat f = surface_capabilities.formats[i];
        if (f == WGPUTextureFormat_BGRA8UnormSrgb || f == WGPUTextureFormat_RGBA8UnormSrgb)
        {
            format = f;
            break;
        }
    }

    WGPUSurfaceConfiguration surface_configuration = {0};
    surface_configuration.nextInChain = NULL;
    surface_configuration.width = 800;
    surface_configuration.height = 600;
    surface_configuration.format = format;
    surface_configuration.viewFormatCount = 0;
    surface_configuration.viewFormats = NULL;
    surface_configuration.usage = WGPUTextureUsage_RenderAttachment;
    surface_configuration.presentMode = WGPUPresentMode_Fifo;
    surface_configuration.alphaMode = WGPUCompositeAlphaMode_Auto;
    surface_configuration.device = device;

    wgpuSurfaceConfigure(surface, &surface_configuration);

    wgpuAdapterRelease(adapter);
    wgpuInstanceRelease(instance);
    
    WGPUShaderSourceWGSL shader_source = {0};
    shader_source.chain.next = NULL;
    shader_source.chain.sType = WGPUSType_ShaderSourceWGSL;
    shader_source.code = (WGPUStringView) { CODE, strlen(CODE) };

    WGPUShaderModuleDescriptor module_descriptor = {0};
    module_descriptor.nextInChain = &shader_source.chain;
    module_descriptor.label = (WGPUStringView) { NULL, SIZE_MAX };

    WGPUShaderModule module = wgpuDeviceCreateShaderModule(device, &module_descriptor);

    WGPURenderPipelineDescriptor pipeline_descriptor = {0};
    pipeline_descriptor.nextInChain = NULL;
    pipeline_descriptor.label = (WGPUStringView) { NULL, SIZE_MAX };
    pipeline_descriptor.layout = NULL;

    pipeline_descriptor.vertex.nextInChain = NULL;
    pipeline_descriptor.vertex.entryPoint = (WGPUStringView) { "vert", 4 };
    pipeline_descriptor.vertex.module = module;
    pipeline_descriptor.vertex.bufferCount = 0;
    pipeline_descriptor.vertex.buffers = NULL;
    pipeline_descriptor.vertex.constantCount = 0;
    pipeline_descriptor.vertex.constants = NULL;

    pipeline_descriptor.primitive.nextInChain = NULL;
    pipeline_descriptor.primitive.topology = WGPUPrimitiveTopology_TriangleList;
    pipeline_descriptor.primitive.stripIndexFormat = WGPUIndexFormat_Undefined;
    pipeline_descriptor.primitive.frontFace = WGPUFrontFace_CCW;
    pipeline_descriptor.primitive.cullMode = WGPUCullMode_None;

    WGPUBlendState blend_state = {0};
    blend_state.color.srcFactor = WGPUBlendFactor_SrcAlpha;
    blend_state.color.dstFactor = WGPUBlendFactor_OneMinusSrcAlpha;
    blend_state.color.operation = WGPUBlendOperation_Add;

    blend_state.alpha.srcFactor = WGPUBlendFactor_Zero;
    blend_state.alpha.dstFactor = WGPUBlendFactor_One;
    blend_state.alpha.operation = WGPUBlendOperation_Add;

    WGPUColorTargetState color_target_state = {0};
    color_target_state.nextInChain = NULL;
    color_target_state.format = format;
    color_target_state.blend = &blend_state;
    color_target_state.writeMask = WGPUColorWriteMask_All;

    WGPUFragmentState fragment_state = {0};
    fragment_state.nextInChain = NULL;
    fragment_state.module = module;
    fragment_state.entryPoint = (WGPUStringView) { "frag", 4 };
    fragment_state.constantCount = 0;
    fragment_state.constants = NULL;
    fragment_state.targetCount = 1;
    fragment_state.targets = &color_target_state;

    pipeline_descriptor.fragment = &fragment_state;

    pipeline_descriptor.multisample.count = 1;
    pipeline_descriptor.multisample.mask = ~0u;
    pipeline_descriptor.multisample.alphaToCoverageEnabled = false;
    
    pipeline_descriptor.depthStencil = NULL;

    WGPURenderPipeline pipeline = wgpuDeviceCreateRenderPipeline(device, &pipeline_descriptor);
    
    wgpuShaderModuleRelease(module);
    
    WGPUSurfaceTexture surface_texture = {0};
    WGPUTextureView target_view = {0};
    while (!glfwWindowShouldClose(window))
    {
        wgpuDevicePoll(device, false, NULL);
        glfwPollEvents();

        wgpuSurfaceGetCurrentTexture(surface, &surface_texture);

        if (surface_texture.status == WGPUSurfaceGetCurrentTextureStatus_SuccessOptimal)
        {
            WGPUTextureViewDescriptor view_descriptor = {0};
            view_descriptor.nextInChain = NULL;
            view_descriptor.label = (WGPUStringView) { NULL, SIZE_MAX };
            view_descriptor.format = wgpuTextureGetFormat(surface_texture.texture);
            view_descriptor.dimension = WGPUTextureViewDimension_2D;
            view_descriptor.baseMipLevel = 0;
            view_descriptor.mipLevelCount = 1;
            view_descriptor.baseArrayLayer = 0;
            view_descriptor.arrayLayerCount = 1;
            view_descriptor.aspect = WGPUTextureAspect_All;
            view_descriptor.usage = WGPUTextureUsage_RenderAttachment;

            target_view = wgpuTextureCreateView(surface_texture.texture, &view_descriptor);
        }

        if (target_view == NULL)
            continue;

        WGPURenderPassDescriptor render_pass_descriptor = {0};
        render_pass_descriptor.nextInChain = NULL;

        WGPURenderPassColorAttachment color_attachment = {0};
        color_attachment.view = target_view;
        color_attachment.resolveTarget = NULL;
        color_attachment.loadOp = WGPULoadOp_Clear;
        color_attachment.storeOp = WGPUStoreOp_Store;
        color_attachment.clearValue = (WGPUColor) { 0.0f, 0.0f, 0.0f, 1.0f };
        render_pass_descriptor.colorAttachmentCount = 1;
        render_pass_descriptor.colorAttachments = &color_attachment;

        render_pass_descriptor.depthStencilAttachment = NULL;
        render_pass_descriptor.timestampWrites = NULL;

        WGPUCommandEncoderDescriptor encoder_descriptor = {0};
        encoder_descriptor.nextInChain = NULL;
        encoder_descriptor.label = (WGPUStringView) { NULL, SIZE_MAX };
        WGPUCommandEncoder encoder = wgpuDeviceCreateCommandEncoder(device, &encoder_descriptor);

        WGPURenderPassEncoder render_pass = wgpuCommandEncoderBeginRenderPass(encoder, &render_pass_descriptor);

        wgpuRenderPassEncoderSetPipeline(render_pass, pipeline);
        wgpuRenderPassEncoderDraw(render_pass, 3, 1, 0, 0);

        wgpuRenderPassEncoderEnd(render_pass);
        wgpuRenderPassEncoderRelease(render_pass);

        WGPUCommandBufferDescriptor command_buffer_descriptor = {0};
        command_buffer_descriptor.nextInChain = NULL;
        command_buffer_descriptor.label = (WGPUStringView) { NULL, SIZE_MAX };

        WGPUCommandBuffer command_buffer = wgpuCommandEncoderFinish(encoder, &command_buffer_descriptor);

        wgpuQueueSubmit(queue, 1, &command_buffer);
        wgpuCommandBufferRelease(command_buffer);

        wgpuTextureViewRelease(target_view);
        target_view = NULL;
        wgpuSurfacePresent(surface);
    }

    wgpuQueueRelease(queue);
    wgpuDeviceRelease(device);
    wgpuSurfaceRelease(surface);
    glfwDestroyWindow(window);
    glfwTerminate();
    exit(EXIT_SUCCESS);
}

void error_callback(int error, const char* description)
{
    fprintf(stderr, "Error: %s\n", description);
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GLFW_TRUE);
}

static const WGPULimits DEFAULT_LIMITS = {
	.nextInChain = NULL,
	.maxTextureDimension1D = WGPU_LIMIT_U32_UNDEFINED,
	.maxTextureDimension2D = WGPU_LIMIT_U32_UNDEFINED,
	.maxTextureDimension3D = WGPU_LIMIT_U32_UNDEFINED,
	.maxTextureArrayLayers = WGPU_LIMIT_U32_UNDEFINED,
	.maxBindGroups = WGPU_LIMIT_U32_UNDEFINED,
	.maxBindGroupsPlusVertexBuffers = WGPU_LIMIT_U32_UNDEFINED,
	.maxBindingsPerBindGroup = WGPU_LIMIT_U32_UNDEFINED,
	.maxDynamicUniformBuffersPerPipelineLayout = WGPU_LIMIT_U32_UNDEFINED,
	.maxDynamicStorageBuffersPerPipelineLayout = WGPU_LIMIT_U32_UNDEFINED,
	.maxSampledTexturesPerShaderStage = WGPU_LIMIT_U32_UNDEFINED,
	.maxSamplersPerShaderStage = WGPU_LIMIT_U32_UNDEFINED,
	.maxStorageBuffersPerShaderStage = WGPU_LIMIT_U32_UNDEFINED,
	.maxStorageTexturesPerShaderStage = WGPU_LIMIT_U32_UNDEFINED,
	.maxUniformBuffersPerShaderStage = WGPU_LIMIT_U32_UNDEFINED,
	.maxUniformBufferBindingSize = WGPU_LIMIT_U64_UNDEFINED,
	.maxStorageBufferBindingSize = WGPU_LIMIT_U64_UNDEFINED,
	.minUniformBufferOffsetAlignment = WGPU_LIMIT_U32_UNDEFINED,
	.minStorageBufferOffsetAlignment = WGPU_LIMIT_U32_UNDEFINED,
	.maxVertexBuffers = WGPU_LIMIT_U32_UNDEFINED,
	.maxBufferSize = WGPU_LIMIT_U64_UNDEFINED,
	.maxVertexAttributes = WGPU_LIMIT_U32_UNDEFINED,
	.maxVertexBufferArrayStride = WGPU_LIMIT_U32_UNDEFINED,
	.maxInterStageShaderVariables = WGPU_LIMIT_U32_UNDEFINED,
	.maxColorAttachments = WGPU_LIMIT_U32_UNDEFINED,
	.maxColorAttachmentBytesPerSample = WGPU_LIMIT_U32_UNDEFINED,
	.maxComputeWorkgroupStorageSize = WGPU_LIMIT_U32_UNDEFINED,
	.maxComputeInvocationsPerWorkgroup = WGPU_LIMIT_U32_UNDEFINED,
	.maxComputeWorkgroupSizeX = WGPU_LIMIT_U32_UNDEFINED,
	.maxComputeWorkgroupSizeY = WGPU_LIMIT_U32_UNDEFINED,
	.maxComputeWorkgroupSizeZ = WGPU_LIMIT_U32_UNDEFINED,
	.maxComputeWorkgroupsPerDimension = WGPU_LIMIT_U32_UNDEFINED
};

WGPULimits get_required_limits(WGPULimits* supported_limits) {
	WGPULimits required_limits = DEFAULT_LIMITS;

	required_limits.maxVertexAttributes           = 3;
	required_limits.maxVertexBuffers              = 1;
	required_limits.maxBufferSize                 = 256 * 1024 * 1024; // 256 MiB
	required_limits.maxVertexBufferArrayStride    = sizeof(float) * 8;
	required_limits.maxInterStageShaderVariables  = 6;

	required_limits.maxBindGroups = 2;
	required_limits.maxUniformBuffersPerShaderStage = 2;
	required_limits.maxUniformBufferBindingSize = 4 * 16 * sizeof(float);
	required_limits.maxSampledTexturesPerShaderStage = 1;
	required_limits.maxSamplersPerShaderStage = 1;

	// require default maximum value for 2D textures
	required_limits.maxTextureDimension1D = 8192;
	required_limits.maxTextureDimension2D = 8192;
	required_limits.maxTextureArrayLayers = 1;

	// explicitly forward minimum limits, since they may cause issues otherwise
	required_limits.minStorageBufferOffsetAlignment = supported_limits->minStorageBufferOffsetAlignment;
	required_limits.minUniformBufferOffsetAlignment = supported_limits->minUniformBufferOffsetAlignment;

	return required_limits;
}

void adapter_callback(
	WGPURequestAdapterStatus status, WGPUAdapter adapter,
	WGPUStringView message, void* userdata1, void* userdata2
) {
	if (status == WGPURequestAdapterStatus_Success) {
		*(WGPUAdapter*)userdata1 = adapter; 
	} else {
		fprintf(stderr, "Failed to get WebGPU adapter: %s\n", message.data);
		return;
	}
	*(bool*)userdata2 = true;
}

void device_callback(
	WGPURequestDeviceStatus status, WGPUDevice device,
	WGPUStringView message, void* userdata1, void* userdata2
) {
	if (status == WGPURequestDeviceStatus_Success) {
		*(WGPUDevice*)userdata1 = device; 
	} else {
		fprintf(stderr, "Failed to get WebGPU device: %s\n", message.data);
		return;
	}
	*(bool*)userdata2 = true;
}

void device_lost_callback(
	const WGPUDevice* device, WGPUDeviceLostReason reason,
	WGPUStringView message, void* userdata1, void* userdata2
) {
	(void)userdata1;
	(void)userdata2;
	fprintf(stderr, "Device 0x%zx lost with reason 0x%zx: %s\n", (size_t)*device, (size_t)reason, message.data);
}

void uncaptured_error_callback(
	const WGPUDevice* device, WGPUErrorType error,
	WGPUStringView message, void* userdata1, void* userdata2
) {
	(void)userdata1;
	(void)userdata2;
	fprintf(stderr, "Device 0x%zx has uncaptured error 0x%zx: %s\n", (size_t)*device, (size_t)error, message.data);
}
