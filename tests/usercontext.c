#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <stdio.h>

static void error_callback(int error, const char* description)
{
    fprintf(stderr, "Error: %s\n", description);
}

int main(void)
{
    GLFWwindow* window;
    GLFWusercontext* usercontext;

    glfwSetErrorCallback(error_callback);

    /* Initialize the library */
    if (!glfwInit())
        return -1;

    /* Create a windowed mode window and its OpenGL context */
    window = glfwCreateWindow(640, 480, "User Context", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        return -1;
    }

    /* Make the window's context current */
    glfwMakeContextCurrent(window);
    gladLoadGL(glfwGetProcAddress);

    /* make a new context */
    usercontext = glfwCreateUserContext(window);
    if (!usercontext)
    {
        fprintf(stderr, "Failed to create user context\n");
        glfwTerminate();
        return -1;
    }


    /* set the user context current */
    glfwMakeUserContextCurrent(usercontext);

    if (glfwGetCurrentContext()!=NULL)
    {
        fprintf(stderr, "Current glfw window context not NULL after glfwMakeUserContextCurrent\n");
        glfwTerminate();
        return -1;
    }
    if (glfwGetCurrentUserContext()!=usercontext)
    {
        fprintf(stderr, "Current user context not correct after glfwMakeUserContextCurrent\n");
        glfwTerminate();
        return -1;
    }

    /* set the window context current */
    glfwMakeContextCurrent(window);

    if ( glfwGetCurrentUserContext() != NULL )
    {
        fprintf(stderr, "Current user context not NULL after glfwMakeContextCurrent\n");
        glfwTerminate();
        return -1;
    }
    if ( glfwGetCurrentContext() != window )
    {
        fprintf(stderr, "Current glfw window context not correct after glfwMakeContextCurrent\n");
        glfwTerminate();
        return -1;
    }

    glClearColor( 0.4f, 0.3f, 0.4f, 1.0f );


    /* Loop until the user closes the window */
    while (!glfwWindowShouldClose(window))
    {
        /* Render here */
        glClear(GL_COLOR_BUFFER_BIT);

        /* Swap front and back buffers */
        glfwSwapBuffers(window);

        /* Poll for and process events */
        glfwPollEvents();
    }

    glfwDestroyUserContext(usercontext);
    glfwTerminate();
    return 0;
}