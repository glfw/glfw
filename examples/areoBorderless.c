#if defined(_MSC_VER)
 // Make MS math.h define M_PI
#define _USE_MATH_DEFINES
#endif

#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define GLAD_GL_IMPLEMENTATION
#include <glad/gl.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <Windows.h>

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
void button_callback(GLFWwindow* window, int button, int action, int mods);

int leftMousePressed = 0;
int leftMouseClicked = 0;
int lestMouseReleased = 0;

void moveWindow(GLFWwindow* window)
{
    int resizeFrameSize = 5;
    int grabBarSize = 20;

    int windowWidth;
    int windowHeight;

    glfwGetWindowSize(window, &windowWidth, &windowHeight);

    int windowPosX;
    int windowPosY;

    glfwGetWindowPos(window, &windowPosX, &windowPosY);

    double mousePositionInWindowX;
    double mousePositionInWindowY;

    glfwGetCursorPos(window, &mousePositionInWindowX, &mousePositionInWindowY);

    static double lastMousePositionInWindowX = 0;
    static double lastMousePositionInWindowY = 0;

    int mouseUpdated = 0;

    if (lastMousePositionInWindowX != mousePositionInWindowX || lastMousePositionInWindowY != mousePositionInWindowY)
    {
        lastMousePositionInWindowX = mousePositionInWindowX;
        lastMousePositionInWindowY = mousePositionInWindowY;
        mouseUpdated = 1;
    }

    static int lockX = 0;
    static int lockY = 0;
    static int grabActive = 0;

    if (leftMouseClicked)
    {
        if (mousePositionInWindowX > resizeFrameSize && mousePositionInWindowX < windowWidth - resizeFrameSize &&
            mousePositionInWindowY > resizeFrameSize && mousePositionInWindowY < grabBarSize + resizeFrameSize)
        {
            lockX = mousePositionInWindowX;
            lockY = mousePositionInWindowY;
            grabActive = 1;
        }
    }
    if (lestMouseReleased)
    {
        grabActive = 0;
    }

    if (grabActive && mouseUpdated)
    {
        int currentX = windowPosX + mousePositionInWindowX - lockX;
        int currentY = windowPosY + mousePositionInWindowY - lockY;

        glfwSetWindowPos(window, currentX, currentY);
    }

}

void resizeWidnow(GLFWwindow* window)
{
    int resizeFrameSize = 4;
    int grabBarSize = 20;

    int windowWidth;
    int windowHeight;

    glfwGetWindowSize(window, &windowWidth, &windowHeight);

    int windowPosX;
    int windowPosY;

    glfwGetWindowPos(window, &windowPosX, &windowPosY);

    double mousePositionInWindowX;
    double mousePositionInWindowY;

    glfwGetCursorPos(window, &mousePositionInWindowX, &mousePositionInWindowY);

    static double lastMousePositionInWindowX = 0;
    static double lastMousePositionInWindowY = 0;

    int mouseUpdated = 0;

    enum resizeType {
        LEFT_TOP = 1 << 0,
        RIGHT_TOP = 1 << 1,
        LEFT_BOTTOM = 1 << 2,
        RIGHT_BOTTOM = 1 << 3,
        LEFT = 1 << 4,
        RIGHT = 1 << 5,
        TOP = 1 << 6,
        BOTTOM = 1 << 7
    };

    enum resizeType type = 0;

    if (mousePositionInWindowX >= 0 && mousePositionInWindowX < resizeFrameSize)
    {
        type |= LEFT;
    }
    if (mousePositionInWindowX <= windowWidth && mousePositionInWindowX > windowWidth - resizeFrameSize)
    {
        type |= RIGHT;
    }
    if (mousePositionInWindowY >= 0 && mousePositionInWindowY < resizeFrameSize)
    {
        type |= TOP;
    }
    if (mousePositionInWindowY <= windowHeight && mousePositionInWindowY > windowHeight - resizeFrameSize)
    {
        type |= BOTTOM;
    }

    if (type == (LEFT & TOP))
    {
        type = LEFT_TOP;
    }
    else if (type == (RIGHT & TOP))
    {
        type = RIGHT_TOP;
    }
    else if (type == (LEFT & BOTTOM))
    {
        type = LEFT_BOTTOM;
    }
    else if (type == (RIGHT & BOTTOM))
    {
        type = RIGHT_BOTTOM;
    }

    //printf("%d", type);
}

/* program entry */
int main(int argc, char* argv[])
{
    GLFWwindow* window;
    int width, height;

    if (!glfwInit())
    {
        fprintf(stderr, "Failed to initialize GLFW\n");
        exit(EXIT_FAILURE);
    }

    glfwWindowHint(GLFW_BORDERLESS_AREO, GLFW_TRUE);

    window = glfwCreateWindow(800, 600, "areoBorderless", NULL, NULL);
    if (!window)
    {
        fprintf(stderr, "Failed to open GLFW window\n");
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    glfwMakeContextCurrent(window);
    glfwSetKeyCallback(window, key_callback);
    glfwSetMouseButtonCallback(window, button_callback);

    glfwSetWindowPos(window, 300, 200);
    glfwSetWindowBorderlessResizeBorderSize(window, 8);
    glfwSetWindowTitle(window, "Hello");

    while (!glfwWindowShouldClose(window))
    {
        int w, h;
        glfwGetWindowSize(window, &w, &h);
       // printf("%d, %d\n", w, h);

        leftMouseClicked = 0;
        lestMouseReleased = 0;

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();

    exit(EXIT_SUCCESS);
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_E && action == GLFW_PRESS)
        glfwIconifyWindow(window);

    if (key == GLFW_KEY_W && action == GLFW_PRESS)
        glfwMaximizeWindow(window);


    if (key == GLFW_KEY_Q && action == GLFW_PRESS)
        glfwRestoreWindow(window);
}

void button_callback(GLFWwindow* window, int button, int action, int mods)
{
    if (button == GLFW_MOUSE_BUTTON_1 && action == GLFW_PRESS)
    {
        leftMouseClicked = 1;
        leftMousePressed = 1;
    }
    else if (button == GLFW_MOUSE_BUTTON_1 && action == GLFW_RELEASE)
    {
        leftMousePressed = 0;
        lestMouseReleased = 1;
    }
}