//========================================================================
// Simple 'Graphics' options test
// Copyright (C) 2015 Eden Salomon <Niko113355@gmail.com>
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
//
// This test creates a window with the ability to change during runtime
// the window options typically seen in the 'Graphics' section of games.
// This includes resolution, refresh rate, and fullscreen / windowed mode.
//
//========================================================================

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#define GLFW_EXPOSE_NATIVE_WGL
#include <GLFW/glfw3native.h>

#include <stdio.h>
#include <stdlib.h>

#include "getopt.h"

typedef struct _WindowState
{
    SIZE* dimenstions;
    int size;
    int count;
    int current;
} WindowState;

static void error_callback(int error, const char* description)
{
    fprintf(stderr, "Error: %s\n", description);
}

static void list_resolutions(GLFWwindow* window)
{
	GLFWmonitor* monitor;
	int count;
	const GLFWvidmode* modes;

	monitor = glfwGetWindowMonitor(window);
	modes = glfwGetVideoModes(monitor ? monitor : glfwGetPrimaryMonitor(), &count);

	printf("List of supported resolutions:\n");
	printf(" +----+------------------------+----+\n");
	printf(" |    |  resolution  |  color  |    |\n");
	printf(" | id | width height | r  g  b | rf |\n");
	printf(" +----+------------------------+----+\n");
	for (int i = 0; i < count; ++i) {
		printf(" | %2d | %5d %6d | %d  %d  %d | %2d |\n",
			   i + 1,
			   modes[i].width, modes[i].height,
			   modes[i].redBits,
			   modes[i].greenBits,
			   modes[i].blueBits,
			   modes[i].refreshRate);
	}
	printf(" +----+------------------------+----+\n\n");
}

static void setVideoMode(GLFWwindow* window, WindowState* state, int index)
{
    GLFWmonitor* monitor;
    const GLFWvidmode* mode;
    int width, height;

    state->current = (index + state->count) % state->count;
    width = state->dimenstions[state->current].cx;
    height = state->dimenstions[state->current].cy;

    glfwSetWindowSize(window, width, height);

    monitor = glfwGetWindowMonitor(window);
    mode = glfwGetVideoMode(monitor);

    printf("Changed resolution to %dx%d [%d].\n", width, height, mode->refreshRate);
}

static void controls()
{
	printf("When the window is in focus, use the following keys to control the various options:\n");
	printf("  Esc    Exit the application\n");
	printf("  h      display this help\n");
	printf("  f      toggle fullscreen / window mode\n");
	printf("  r      toggle between different refresh rates\n");
	printf("  l      list all supported resolutions\n");
	printf("  d, ->  next resolution\n");
	printf("  a, <-  previous resolution\n");
}

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    WindowState* state = (WindowState*) glfwGetWindowUserPointer(window);

    if (action != GLFW_PRESS)
        return;

    switch (key)
    {
        case GLFW_KEY_H:
            controls();
            break;
        case GLFW_KEY_L:
            list_resolutions(window);
            break;
        case GLFW_KEY_F:
            glfwToggleWindowFullscreen(window);
            break;
        case GLFW_KEY_ESCAPE:
            glfwSetWindowShouldClose(window, GLFW_TRUE);
            break;
        case GLFW_KEY_A:
            setVideoMode(window, state, state->current - 1);
            break;
        case GLFW_KEY_D:
            setVideoMode(window, state, state->current + 1);
            break;
        case GLFW_KEY_R:
            // TODO: toggle refresh rate on current resolution
            // as of now the highest value is used due to
            // GLFW_REFRESH_RATE being set to GLFW_DONT_CARE
            break;
    }
}

static void usage()
{
    printf("Usage: oprions [OPTION]...\n");
    printf("Options:\n");
    printf("  -h, --help                show this help\n");
    printf("  -l, --list-resolutions    list all resolutions supported by the primary monitor\n");
    printf("      --width=640           specify the width of the window in pixels\n");
    printf("      --height=480          specify the height of the window in pixels\n");
    printf("  -r, --refresh-rate=RATE   specify the refresh rate of the window in Hz\n");
}

int main(int argc, char** argv)
{
    int ch;
	int width = 640;
	int height = 480;
	int list = GLFW_FALSE;
	int fullscreen = GLFW_FALSE;
    int running = GLFW_TRUE;
	GLFWwindow* window;
	GLFWmonitor* monitor;
    WindowState state = { NULL, 0, 0, 0 };
	int i, j, count;
	const GLFWvidmode* modes;

	enum { HELP, RESOLUTIONS, WIDTH, HIEGHT, REFRESH, FULLSCREEN };

	const struct option options[] =
    {
        { "help",				0, NULL, HELP },
		{ "list-resolutions",	0, NULL, RESOLUTIONS },
		{ "width",				1, NULL, WIDTH },
		{ "height",				1, NULL, HIEGHT },
		{ "refresh-rate",		1, NULL, REFRESH },
		{ "fullscreen",		    0, NULL, FULLSCREEN },
        { NULL, 0, NULL, 0 }
    };

    glfwSetErrorCallback(error_callback);

    if (!glfwInit())
        exit(EXIT_FAILURE);

	while ((ch = getopt_long(argc, argv, "hlr:f", options, NULL)) != -1) {
		switch (ch) {
			case 'h':
			case HELP:
				usage();
				exit(EXIT_SUCCESS);
			case 'l':
			case RESOLUTIONS:
				list = GLFW_TRUE;
				break;
			case WIDTH:
				width = atoi(optarg);
				break;
			case HIEGHT:
				height = atoi(optarg);
				break;
			case 'r':
			case REFRESH:
				glfwWindowHint(GLFW_REFRESH_RATE, atoi(optarg));
				break;
			case 'f':
			case FULLSCREEN:
				glfwWindowHint(GLFW_FULLSCREEN, GLFW_TRUE);
				break;
			default:
                usage();
                exit(EXIT_FAILURE);
		}
	}

	monitor = glfwGetPrimaryMonitor();
    window = glfwCreateWindow(width, height, "Graphics Options", (fullscreen ? monitor : NULL), NULL);
    
	modes = glfwGetVideoModes(monitor ? monitor : glfwGetPrimaryMonitor(), &count);

    for (i = 0; i < count; ++i) {
        SIZE size;

        size.cx = modes[i].width;
        size.cy = modes[i].height;

        for (j = 0; j < state.count; ++j) {
            if (state.dimenstions[j].cx == size.cx &&
                state.dimenstions[j].cy == size.cy)
                break;
        }
        
        if (j < state.count)
            continue;

        if (state.count == state.size)
        {
            if (state.count)
                state.size *= 2;
            else
                state.size = 128;

            state.dimenstions = (SIZE*) realloc(state.dimenstions, state.size * sizeof(SIZE));
        }

        state.count++;
        state.dimenstions[state.count - 1] = size;
    }

    glfwSetWindowUserPointer(window, &state);

	if (!window) {
		glfwTerminate();
		exit(EXIT_FAILURE);
	}
    
	glfwSetKeyCallback(window, key_callback);
    
	glfwMakeContextCurrent(window);
    
	gladLoadGLLoader((GLADloadproc) glfwGetProcAddress);
	
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	
	if (list)
		list_resolutions(window);

	controls();

    while (running)
    {
        glClear(GL_COLOR_BUFFER_BIT);
        glfwSwapBuffers(window);

        if (glfwWindowShouldClose(window))
            running = GLFW_FALSE;

        glfwPollEvents();
    }

    free(state.dimenstions);
    glfwTerminate();
    exit(EXIT_SUCCESS);
}

