
#define GLAD_GL_IMPLEMENTATION
#include <glad/gl.h>

#define GLFW_INCLUDE_NONE
#define GLFW_EXPOSE_NATIVE_WAYLAND
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>


#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>


#define WIN_HEIGHT 150


static void GLFW_DebugCallback(int err_code, const char* description)
{
    printf("GLFW error: %s\n", description);
}


static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    static bool onTopLayer  = true;
    static bool onTopAnchor = true;

    if (action == GLFW_PRESS)
    {
        printf("Grabbing...\n");
        switch (key)
        {
            case GLFW_KEY_RIGHT:
            case GLFW_KEY_LEFT :
            {
                onTopLayer = !onTopLayer;

                if (onTopLayer)
                {
                    printf("Send to top\n");
                    glfwWaylandZwlrSetLayer(window, GLFW_WAYLAND_ZWLR_LAYER_TOP);
                }
                else
                {
                    printf("Send to bottom\n");
                    glfwWaylandZwlrSetLayer(window, GLFW_WAYLAND_ZWLR_LAYER_BOTTOM);
                }

                break;
            }

            case GLFW_KEY_UP:
            case GLFW_KEY_DOWN:
            {
                onTopAnchor = !onTopAnchor;

                if (onTopAnchor)
                {
                    printf("Stick to top\n");
                    glfwWaylandZwlrSetAnchor(window, GLFW_WAYLAND_ZWLR_ANCHOR_TOP);

                }
                else
                {
                    printf("Stick to bottom\n");
                    glfwWaylandZwlrSetAnchor(window, GLFW_WAYLAND_ZWLR_ANCHOR_BOTTOM);
                }
                break;
            }
        }
    }
}


void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
    if (action == GLFW_RELEASE)
        printf("click\n");
}


int main(int argc, char** argv)
{
    glfwSetErrorCallback(GLFW_DebugCallback);

    if (!glfwInit()) return 1;

    glfwWindowHint(GLFW_WAYLAND_USE_ZWLR, GLFW_WAYLAND_ZWLR_LAYER_TOP);

    int monitorX_pos, monitorY_pos;
    int monitorWidth, monitorHeight;
    GLFWmonitor* primary_monitor = glfwGetPrimaryMonitor();
    glfwGetMonitorWorkarea(primary_monitor, &monitorX_pos, &monitorY_pos, &monitorWidth, &monitorHeight);

                                        // v-- 0 not allowed by glfw api, for avoiding mess with internal api, you are forced..
                                        // v   ..to request monitor size and manually handle window size
    GLFWwindow* window = glfwCreateWindow(300, WIN_HEIGHT, "Don't Care", NULL, NULL);
    if (!window) return 1;

    glfwSetWindowSize(window, monitorWidth, WIN_HEIGHT); // just testing
    //glfwWaylandZwlrSetExclusiveZone(window, WIN_HEIGHT + 50); // try to play with
    //glfwWaylandZwlrSetMargin(window, 10, 10, 10, 10);

    glfwMakeContextCurrent    (window);
    gladLoadGL(glfwGetProcAddress);
    glfwSetMouseButtonCallback(window, mouse_button_callback   );
    //glfwSetCursorPosCallback  (window, cursor_position_callback);
    glfwSetKeyCallback        (window, key_callback            );
    //glfwSetWindowFocusCallback(window, window_focus_callback)  ;


    glClearColor(0.3f, 0.2f, 0.3f, 1.0f);

    while (!glfwWindowShouldClose(window))
    {
        glfwSwapBuffers(window);
        glfwPollEvents();

        glClear(GL_COLOR_BUFFER_BIT);
    }

    return 0;
}



