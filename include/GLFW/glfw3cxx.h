

namespace glfw
{
    namespace
    {
        #include"glfw3.h"
    }

    GLFWAPI int (&Init)(void) = glfwInit;
    GLFWAPI void (&WindowHint)(int, int) = glfwWindowHint;

}
