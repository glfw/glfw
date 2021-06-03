#include <GLFW/glfw3.h>

#include <iostream>

void printWindowTitle(GLFWwindow *window)
{
	std::cout << "The window title should be '" << glfwGetWindowTitle(window) << "'.\n";
}

void windowShow(GLFWwindow *window)
{
	printWindowTitle(window);
	while (!glfwWindowShouldClose(window))
	{
		glfwWaitEvents();
	}
	glfwSetWindowShouldClose(window, GLFW_FALSE);
}

int main()
{
	if (!glfwInit())
	{
		std::cerr << "Could not initialise glfw.\n";
		return -1;
	}

	GLFWwindow *window = glfwCreateWindow(800, 600, "Initial title", NULL, NULL);
	windowShow(window);

	glfwSetWindowTitle(window, "");
	windowShow(window);

	glfwSetWindowTitle(window, "Potato's are cool");
	windowShow(window);

	glfwSetWindowTitle(window, u8"😀 😃 😄 😁");
	windowShow(window);

	glfwDestroyWindow(window);

	window = glfwCreateWindow(800, 600, "", NULL, NULL);
	windowShow(window);
	
	glfwTerminate();

	return 0;
}