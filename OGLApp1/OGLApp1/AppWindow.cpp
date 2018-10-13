#include "AppWindow.h"
#include <iostream>


AppWindow::AppWindow()
{
	width = 800;
	height = 600;
}

AppWindow::AppWindow(GLint window_width, GLint window_height) {
	width = window_width;
	height = window_height;
}

int AppWindow::Init() {
	/// Initialise GLFW (windows)
	if (!glfwInit()) {
		std::cout << "GLFW init failed!";
		glfwTerminate();
		return 1; //TODO ERRORS
	}

	/// Setup GLFW window properties, create it and get buffersize
	// OpenGL version X.Y
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4); //X
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6); //Y
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); //Not b-w compatible
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // F-w compatible
	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);

	// Create main window
	main_window = glfwCreateWindow(width, height, "OGLAPP1", NULL, NULL);
	if (!main_window) { std::cerr << "Main GLFW window null."; glfwTerminate(); return 2; }

	// Framebuffer size
	glfwGetFramebufferSize(main_window, &buffer_width, &buffer_height);

	//GLFW context to GLFW window
	glfwMakeContextCurrent(main_window);

	///GLEW init
	//Extensions
	glewExperimental = GL_TRUE;

	if (glewInit() != GLEW_OK) {
		std::cerr << "GLEW init failed!";
		glfwDestroyWindow(main_window);
		glfwTerminate();
		return 3;
	}

	//enable depth test
	glEnable(GL_DEPTH_TEST);

	// Set viewport to window size
	glViewport(0, 0, buffer_width, buffer_height);
}

AppWindow::~AppWindow()
{
	glfwDestroyWindow(main_window);
	glfwTerminate();
}
