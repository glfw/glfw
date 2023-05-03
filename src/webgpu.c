//========================================================================
// GLFW 3.4 - www.glfw.org
//------------------------------------------------------------------------
// Copyright (c) 2022-2023 Elie Michel <eliemichelfr@gmail.com>  and the
// wgpu-native authors.
// Most of the code from this file comes from the wgpu-native triangle example:
//   https://github.com/gfx-rs/wgpu-native/blob/master/examples/triangle/main.c
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
// Please use C89 style variable declarations in this file because VS 2010
//========================================================================

#if defined(_GLFW_BUILD_WEBGPU)

#include <webgpu/webgpu.h>

#define WGPU_TARGET_MACOS 1
#define WGPU_TARGET_LINUX_X11 2
#define WGPU_TARGET_WINDOWS 3
#define WGPU_TARGET_LINUX_WAYLAND 4

#if defined(_WIN32)
#define WGPU_TARGET WGPU_TARGET_WINDOWS
#elif defined(__APPLE__)
#define WGPU_TARGET WGPU_TARGET_MACOS
#elif defined(_GLFW_WAYLAND)
#define WGPU_TARGET WGPU_TARGET_LINUX_WAYLAND
#else
#define WGPU_TARGET WGPU_TARGET_LINUX_X11
#endif

#if WGPU_TARGET == WGPU_TARGET_MACOS
#include <Foundation/Foundation.h>
#include <QuartzCore/CAMetalLayer.h>
#endif

#include <GLFW/glfw3.h>
#if WGPU_TARGET == WGPU_TARGET_MACOS
#define GLFW_EXPOSE_NATIVE_COCOA
#elif WGPU_TARGET == WGPU_TARGET_LINUX_X11
#define GLFW_EXPOSE_NATIVE_X11
#elif WGPU_TARGET == WGPU_TARGET_LINUX_WAYLAND
#define GLFW_EXPOSE_NATIVE_WAYLAND
#elif WGPU_TARGET == WGPU_TARGET_WINDOWS
#define GLFW_EXPOSE_NATIVE_WIN32
#endif
#include <GLFW/glfw3native.h>

//////////////////////////////////////////////////////////////////////////
//////                        GLFW public API                       //////
//////////////////////////////////////////////////////////////////////////

GLFWAPI WGPUSurface glfwCreateWindowWGPUSurface(WGPUInstance instance, GLFWwindow* window) {
#if WGPU_TARGET == WGPU_TARGET_MACOS
    {
        id metal_layer = NULL;
        NSWindow* ns_window = glfwGetCocoaWindow(window);
        [ns_window.contentView setWantsLayer : YES] ;
        metal_layer = [CAMetalLayer layer];
        [ns_window.contentView setLayer : metal_layer] ;
        return wgpuInstanceCreateSurface(
            instance,
            &(WGPUSurfaceDescriptor){
            .label = NULL,
                .nextInChain =
                (const WGPUChainedStruct*)&(
                    WGPUSurfaceDescriptorFromMetalLayer) {
                .chain =
                    (WGPUChainedStruct){
                        .next = NULL,
                        .sType = WGPUSType_SurfaceDescriptorFromMetalLayer,
                },
                .layer = metal_layer,
            },
        });
    }
#elif WGPU_TARGET == WGPU_TARGET_LINUX_X11
    {
        Display* x11_display = glfwGetX11Display();
        Window x11_window = glfwGetX11Window(window);
        return wgpuInstanceCreateSurface(
            instance,
            &(WGPUSurfaceDescriptor){
            .label = NULL,
                .nextInChain =
                (const WGPUChainedStruct*)&(
                    WGPUSurfaceDescriptorFromXlibWindow) {
                .chain =
                    (WGPUChainedStruct){
                        .next = NULL,
                        .sType = WGPUSType_SurfaceDescriptorFromXlibWindow,
                },
                .display = x11_display,
                .window = x11_window,
            },
        });
    }
#elif WGPU_TARGET == WGPU_TARGET_LINUX_WAYLAND
    {
        struct wl_display* wayland_display = glfwGetWaylandDisplay();
        struct wl_surface* wayland_surface = glfwGetWaylandWindow(window);
        return wgpuInstanceCreateSurface(
            instance,
            &(WGPUSurfaceDescriptor){
            .label = NULL,
                .nextInChain =
                (const WGPUChainedStruct*)&(
                    WGPUSurfaceDescriptorFromWaylandSurface) {
                .chain =
                    (WGPUChainedStruct){
                        .next = NULL,
                        .sType =
                            WGPUSType_SurfaceDescriptorFromWaylandSurface,
},
.display = wayland_display,
.surface = wayland_surface,
                },
        });
  }
#elif WGPU_TARGET == WGPU_TARGET_WINDOWS
    {
        HWND hwnd = glfwGetWin32Window(window);
        HINSTANCE hinstance = GetModuleHandle(NULL);
        return wgpuInstanceCreateSurface(
            instance,
            &(WGPUSurfaceDescriptor){
            .label = NULL,
                .nextInChain =
                (const WGPUChainedStruct*)&(
                    WGPUSurfaceDescriptorFromWindowsHWND) {
                .chain =
                    (WGPUChainedStruct){
                        .next = NULL,
                        .sType = WGPUSType_SurfaceDescriptorFromWindowsHWND,
            },
            .hinstance = hinstance,
            .hwnd = hwnd,
        },
    });
  }
#else
#error "Unsupported WGPU_TARGET"
#endif
}
#endif /* _GLFW_BUILD_WEBGPU */
