// compute shaders tutorial
// Dr Anton Gerdelan <gerdela@scss.tcd.ie>
// Trinity College Dublin, Ireland
// 26 Feb 2016

#pragma once
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <assert.h>
#include <stdio.h>

// window dimensions
#define WINDOW_W 512
#define WINDOW_H 512
#ifndef bool
#define bool int
#endif
#ifndef false
#define false 0
#endif

#ifndef true
#define true 1
#endif

bool start_gl();
void stop_gl();
bool check_shader_errors( GLuint shader );
bool check_program_errors( GLuint program );
GLuint create_quad_vao();
GLuint create_quad_program();

extern GLFWwindow *window;
