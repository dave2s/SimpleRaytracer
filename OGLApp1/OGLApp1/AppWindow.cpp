#include "AppWindow.h"
#include <iostream>

//DELETE THESE AFTER PRINTS
//#include <string>

#define NUM_KEYS 1024

AppWindow::AppWindow()
{
	width = 800;
	height = 600;
	KeysInit();
}

AppWindow::AppWindow(GLint window_width, GLint window_height) {
	width = window_width;
	height = window_height;
	KeysInit();
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

	glfwSetWindowUserPointer(main_window, this);

	CreateCallbacks();

	return 0;
}

void AppWindow::KeysInit()
{
	keys.reserve(NUM_KEYS);
	for (int i = 0; i < NUM_KEYS; ++i) {
		keys.push_back(false);
	}
}

void AppWindow::CreateCallbacks()
{
	glfwSetKeyCallback(main_window, HandleKeys);
}

void AppWindow::HandleKeys(GLFWwindow * window, int key, int code, int action, int mode)
{
	AppWindow* _window = static_cast<AppWindow*>(glfwGetWindowUserPointer(window));

	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
	{
		glfwSetWindowShouldClose(window, GL_TRUE);
	}
	if (key >= 0 && key < NUM_KEYS)
	{
		if (action == GLFW_PRESS)
		{
			_window->keys[key] = true;
			//std::cout << "Pressed: "<< std::to_string(key) <<"\n";
		}
		else if (action == GLFW_RELEASE)
		{
			_window->keys[key] = false;
			//std::cout << "Released: " << std::to_string(key) << "\n";
		}
	}
}

AppWindow::~AppWindow()
{
	glfwDestroyWindow(main_window);
	glfwTerminate();
}