#pragma once
#include <stdio.h>
#include <GL\glew.h>
#include <GLFW\glfw3.h>

class AppWindow
{
public:
	AppWindow();
	AppWindow(GLint window_width, GLint window_height);

	int Init();

	GLint getBufferWidth() { return buffer_width; }
	GLint getBufferHeight() { return buffer_height; }

	bool shouldClose() { return glfwWindowShouldClose(main_window); }

	void swapBuffers() { glfwSwapBuffers(main_window); }

	~AppWindow();

private:
	GLFWwindow *main_window;
	GLint width, height;
	GLint buffer_width, buffer_height;
};

