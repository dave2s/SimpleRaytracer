#pragma once
#include <vector>
#include <tuple>
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
	std::vector<bool> getKeys() { return keys; }
	std::tuple<GLfloat, GLfloat> getMouseChange();
	bool shouldClose() { return glfwWindowShouldClose(main_window); }

	void SwapBuffers() { glfwSwapBuffers(main_window); }

	~AppWindow();

private:
	GLFWwindow *main_window;
	GLint width, height;
	GLint buffer_width, buffer_height;
	GLfloat mouse_x, mouse_y, mouse_delta_x, mouse_delta_y;

	std::vector<bool> keys;
	bool mouse_moved = false;

	void KeysInit();

	static void HandleKeysCB(GLFWwindow* window, int key, int code, int action, int mode);
	static void HandleMouseCB(GLFWwindow* window, double x_pos, double y_pos);
	void CreateCallbacks();
};

