#--------------------------------------------------------------------
# Add common sources
#--------------------------------------------------------------------

dependencies = []

sources = ['internal.h',
           'mappings.h',
           '../include/GLFW/glfw3.h',
           '../include/GLFW/glfw3native.h',
           'context.c',
           'init.c',
           'input.c',
           'monitor.c',
           'vulkan.c',
           'window.c']

if host_system == 'linux' or host_system == 'bsd'
    dependencies += [rt_dep, math_dep, cc.find_library('dl', required: false)]
endif


#--------------------------------------------------------------------
# Create generated files
#--------------------------------------------------------------------

conf_data = configuration_data()
foreach b : ['win32',
             'x11',
             'wayland',
             'mir',
             'osmesa',
             'cocoa']
    conf_data.set('_GLFW_'+b.to_upper(), backend == b)
endforeach
conf_data.set('_GLFW_BUILD_DLL', get_option('default_library') == 'shared')
conf_data.set('_GLFW_VULKAN_STATIC', get_option('glfw_vulkan_static'))
conf_data.set('_GLFW_USE_HYBRID_HPG', get_option('glfw_use_hybrid_hpg'))
conf_file = configure_file(configuration: conf_data,
                           output: 'glfw_config.h')
sources += conf_file

#configure_file(input:  'src/glfw3.pc.in',
#               output: 'src/glfw3.pc',
#               configuration: conf_data)



#--------------------------------------------------------------------
# Add multithreading sources
#--------------------------------------------------------------------

dependencies += threads_dep

if host_system == 'windows'
    sources += ['win32_thread.c']
else
    sources += ['posix_thread.h',
                'posix_thread.c']
endif


#--------------------------------------------------------------------
# Add joystick sources
#--------------------------------------------------------------------

if host_system == 'windows'
    sources += ['win32_joystick.h',
                'win32_joystick.c']
elif host_system == 'darwin'
    sources += ['cocoa_joystick.h',
                'cocoa_joystick.c']
elif host_system == 'linux'
    sources += ['linux_joystick.h',
                'linux_joystick.c']
else
    sources += ['null_joystick.h',
                'null_joystick.c']
endif


#--------------------------------------------------------------------
# Add time sources
#--------------------------------------------------------------------

if host_system == 'windows'
    sources += ['win32_time.c']
elif host_system == 'darwin'
    sources += ['cocoa_time.c']
else
    sources += ['posix_time.h',
                'posix_time.c']
endif


#--------------------------------------------------------------------
# Add OpenGL context sources
#--------------------------------------------------------------------

sources += ['osmesa_context.h',
            'osmesa_context.c']

if backend != 'osmesa'
    sources += ['egl_context.h',
                'egl_context.c']
endif

if backend == 'win32'
    sources += ['wgl_context.h',
                'wgl_context.c']
elif backend == 'x11'
    sources += ['glx_context.h',
                'glx_context.c']
elif backend == 'cocoa'
    sources += ['nsgl_context.h',
                'nsgl_context.m']
endif


#--------------------------------------------------------------------
# Add Vulkan sources
#--------------------------------------------------------------------

dependencies += [dependency('vulkan', required: false)] # TODO
# # Dependencies required by the MoltenVK static library
# set(GLFW_VULKAN_DEPS
#     "-lc++"
#     "-framework Cocoa"
#     "-framework Metal"
#     "-framework QuartzCore")

if get_option('glfw_vulkan_static')
    #if (VULKAN_FOUND AND VULKAN_STATIC_LIBRARY)
    #    list(APPEND glfw_LIBRARIES "${VULKAN_STATIC_LIBRARY}" ${GLFW_VULKAN_DEPS})
    #    if (BUILD_SHARED_LIBS)
    #        message(WARNING "Linking Vulkan loader static library into GLFW")
    #    endif()
    #else()
    #    if (BUILD_SHARED_LIBS OR GLFW_BUILD_EXAMPLES OR GLFW_BUILD_TESTS)
    #        message(FATAL_ERROR "Vulkan loader static library not found")
    #    else()
    #        message(WARNING "Vulkan loader static library not found")
    #    endif()
    #endif()
endif


#--------------------------------------------------------------------
# Add backend sources
#--------------------------------------------------------------------

if backend == 'win32'
    dependencies += cc.find_library('gdi32')
    sources += ['win32_init.c',
                'win32_platform.h',
                'win32_monitor.c',
                'win32_window.c']
elif backend == 'x11'
    dependencies += dependency('x11')
    # TODO: XRandR header found?
    # TODO: Xinerama header found?
    # TODO: Xkb header found?
    # TODO: Xcursor header found?
    sources += ['x11_init.c',
                'x11_platform.h',
                'x11_monitor.c',
                'x11_window.c']
elif backend == 'wayland'
    dependencies += [dependency('ecm'),
                     dependency('wayland'),
                     dependency('wayland-scanner'),
                     dependency('wayland-protocols', version: '1.1'),
                     dependency('wayland-egl')]
    sources += ['wl_init.c',
                'wl_platform.h',
                'wl_monitor.c',
                'wl_window.c']
    #ecm_add_wayland_client_protocol(glfw_SOURCES
    #    PROTOCOL
    #    "${WAYLAND_PROTOCOLS_PKGDATADIR}/unstable/relative-pointer/relative-pointer-unstable-v1.xml"
    #    BASENAME relative-pointer-unstable-v1)
    #ecm_add_wayland_client_protocol(glfw_SOURCES
    #    PROTOCOL
    #    "${WAYLAND_PROTOCOLS_PKGDATADIR}/unstable/pointer-constraints/pointer-constraints-unstable-v1.xml"
    #    BASENAME pointer-constraints-unstable-v1)
elif backend == 'mir'
    dependencies += [dependency('mirclient')]
    sources += ['mir_init.c',
                'mir_platform.h',
                'mir_monitor.c',
                'mir_window.c']
elif backend == 'osmesa'
    dependencies += dependency('osmesa')
    sources += ['null_init.c',
                'null_platform.h',
                'null_monitor.c',
                'null_window.c']
elif backend == 'cocoa'
    #list(APPEND glfw_LIBRARIES
    #    "-framework Cocoa"
    #    "-framework IOKit"
    #    "-framework CoreFoundation"
    #    "-framework CoreVideo")
    sources += ['cocoa_init.m',
                'cocoa_platform.h',
                'cocoa_monitor.m',
                'cocoa_window.m']
endif


#--------------------------------------------------------------------
# Add other sources
#--------------------------------------------------------------------

if host_system == 'linux' and backend != 'osmesa'
    if backend != 'x11' # X11 already includes xkbcommon TODO really???
        dependencies += dependency('xkbcommon')
    endif
    sources += ['xkb_unicode.h',
                'xkb_unicode.c']
endif


#--------------------------------------------------------------------
# Declare library and its dependency object
#--------------------------------------------------------------------

glfw_library = library('glfw',
                       sources,
                       version: meson.project_version(),
                       c_args: ['-D_GLFW_USE_CONFIG_H'],
                       dependencies: dependencies,
                       include_directories: [include_directories('../include'),
                                             include_directories('.')])
glfw_dep = declare_dependency(link_with: glfw_library,
                              include_directories:
                                    [include_directories('../include'),
                                     include_directories('.')])


# vim: set softtabstop=4 shiftwidth=4:
