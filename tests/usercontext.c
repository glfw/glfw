#include <glad/gl.h>
#include <GLFW/glfw3.h>

int main(void)
{
    GLFWwindow* window;
    GLFWusercontext* usercontext;

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
        glfwTerminate();
        return -1;
    }

    /* set the user context current */
    glfwMakeUserContextCurrent(usercontext);

    /* set the window context current */
    glfwMakeContextCurrent(window);

    glClearColor( 0.4f, 0.3f, 0.4f, 0.0f );


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