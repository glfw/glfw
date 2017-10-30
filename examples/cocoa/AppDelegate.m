
//========================================================================
// Simple GLFW in Cocoa app example
// Copyright (c) Andy Somogyi <andy.somogyi@gmail.com>
//
// This is a port of the GLFW basic example, originaly 
// Copyright (c) Camilla Löwy <elmindreda@glfw.org>
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

#import "AppDelegate.h"
#include <stdio.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#define GLFW_EXPOSE_NATIVE_COCOA
#include <GLFW/glfw3native.h>

#include "linmath.h"

static void createCocoaGlfwWindow(int width, int height, int xpos, int ypos);
static void cocoaGlfwStep();
static void cocoaGlfwCloseWindow();

@implementation AppDelegate

@synthesize window = _windows;

- (void)applicationDidFinishLaunching:(NSNotification *)aNotification {
  // Insert code here to initialize your application
}

-(IBAction)createGlfwWindow:(id)sender {
    NSRect frame = self.window.frame;
    
    createCocoaGlfwWindow(700, 400, frame.size.width, 0);
}

-(IBAction)singleStep:(id)sender {
    printf("step message \n");
    fflush(stdout);
    cocoaGlfwStep();
}

-(IBAction)closeGlfwWindow:(id)sender {
    printf("close window message \n");
    fflush(stdout);
    cocoaGlfwCloseWindow();
}

-(IBAction)run:(id)sender {
    
    if(self.stepTimer) {
        return;
    }
    
    [self.stepTimer invalidate];
    
    NSTimer *timer = [NSTimer scheduledTimerWithTimeInterval:0.1
                              target:self selector:@selector(singleStep:)
                              userInfo:nil repeats:YES];
    self.stepTimer = timer;
    
}

-(IBAction)stop:(id)sender {
    [self.stepTimer invalidate];
    self.stepTimer = nil;
}

@end


static const struct
{
    float x, y;
    float r, g, b;
} vertices[3] =
{
    { -0.6f, -0.4f, 1.f, 0.f, 0.f },
    {  0.6f, -0.4f, 0.f, 1.f, 0.f },
    {   0.f,  0.6f, 0.f, 0.f, 1.f }
};

static const char* vertex_shader_text =
"#version 110\n"
"uniform mat4 MVP;\n"
"attribute vec3 vCol;\n"
"attribute vec2 vPos;\n"
"varying vec3 color;\n"
"void main()\n"
"{\n"
"    gl_Position = MVP * vec4(vPos, 0.0, 1.0);\n"
"    color = vCol;\n"
"}\n";

static const char* fragment_shader_text =
"#version 110\n"
"varying vec3 color;\n"
"void main()\n"
"{\n"
"    gl_FragColor = vec4(color, 1.0);\n"
"}\n";

static GLuint vertex_buffer, vertex_shader, fragment_shader, program;
static GLint mvp_location, vpos_location, vcol_location;
static GLFWwindow *win;
static float angle = 0;

static void error_callback(int error, const char* description)
{
    fprintf(stderr, "Error: %s\n", description);
}

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GLFW_TRUE);
}

static void window_refresh_callback(GLFWwindow* window)
{
    float ratio;
    int width, height;
    mat4x4 m, p, mvp;
    
    glfwGetFramebufferSize(window, &width, &height);
    ratio = width / (float) height;
    
    glViewport(0, 0, width, height);
    glClear(GL_COLOR_BUFFER_BIT);
    
    mat4x4_identity(m);
    mat4x4_rotate_Z(m, m, angle);
    mat4x4_ortho(p, -ratio, ratio, -1.f, 1.f, 1.f, -1.f);
    mat4x4_mul(mvp, p, m);
    
    glUseProgram(program);
    glUniformMatrix4fv(mvp_location, 1, GL_FALSE, (const GLfloat*) mvp);
    glDrawArrays(GL_TRIANGLES, 0, 3);
    
    glfwSwapBuffers(window);
}

static void window_close_callback(GLFWwindow* window)
{
    glfwDestroyWindow(window);
    win = NULL;
}

static void createCocoaGlfwWindow(int width, int height, int xpos, int ypos) {
    glfwSetErrorCallback(error_callback);
    
    if(win) {
        printf("window is already open\n");
        fflush(stdout);
        return;
    }
    
    if (!glfwInit())
        exit(EXIT_FAILURE);
    
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    
    win = glfwCreateWindow(width, height, "Simple example", NULL, NULL);
    if (!win)
    {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }
    
    glfwSetKeyCallback(win, key_callback);
    
    glfwSetWindowRefreshCallback(win, window_refresh_callback);
    
    glfwSetWindowCloseCallback(win, window_close_callback);
    
    glfwMakeContextCurrent(win);
    gladLoadGLLoader((GLADloadproc) glfwGetProcAddress);
    glfwSwapInterval(1);
    
    // NOTE: OpenGL error checks have been omitted for brevity
    
    glGenBuffers(1, &vertex_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    
    vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex_shader, 1, &vertex_shader_text, NULL);
    glCompileShader(vertex_shader);
    
    fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment_shader, 1, &fragment_shader_text, NULL);
    glCompileShader(fragment_shader);
    
    program = glCreateProgram();
    glAttachShader(program, vertex_shader);
    glAttachShader(program, fragment_shader);
    glLinkProgram(program);
    
    mvp_location = glGetUniformLocation(program, "MVP");
    vpos_location = glGetAttribLocation(program, "vPos");
    vcol_location = glGetAttribLocation(program, "vCol");
    
    glEnableVertexAttribArray(vpos_location);
    glVertexAttribPointer(vpos_location, 2, GL_FLOAT, GL_FALSE,
                          sizeof(vertices[0]), (void*) 0);
    glEnableVertexAttribArray(vcol_location);
    glVertexAttribPointer(vcol_location, 3, GL_FLOAT, GL_FALSE,
                          sizeof(vertices[0]), (void*) (sizeof(float) * 2));
    
}
    
static void cocoaGlfwCloseWindow() {
    glfwDestroyWindow(win);
    win = NULL;
}

static void cocoaGlfwStep() {
    angle += 0.1;
    window_refresh_callback(win);
}
