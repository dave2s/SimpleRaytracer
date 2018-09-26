#include <iostream>
#include <stdio.h>
#include "GL/glew.h"
#include "GLFW/glfw3.h"

const GLint WIDTH = 1600, HEIGHT = 1200;

int main()
{
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

	// Create main window
	GLFWwindow *mainWindow = glfwCreateWindow(WIDTH, HEIGHT, "OGLAPP1", NULL, NULL);
	if (!mainWindow) { std::cerr << "Main GLFW window null."; glfwTerminate(); return 2; }

	// Framebuffer size
	int bufferWidth, bufferHeight;
	glfwGetFramebufferSize(mainWindow, &bufferWidth, &bufferHeight);

	//GLFW context to GLFW window
	glfwMakeContextCurrent(mainWindow);

	///GLEW init
	//Extensions
	glewExperimental = GL_TRUE;

	if (glewInit() != GLEW_OK) {
		std::cerr << "GLEW init failed!";
		glfwDestroyWindow(mainWindow);
		glfwTerminate();
		return 3;
	}

	// Set viewport to window size
	glViewport(0, 0, bufferWidth, bufferHeight);

	//Window loop
	while (!glfwWindowShouldClose(mainWindow)) {
		glfwPollEvents(); // So we can read events later in the loop

		/// So we can draw next frame
		glClearColor(GLclampf(1.f), GLclampf(1.f), GLclampf(1.f), GLclampf(1.f));
		glClear(GL_COLOR_BUFFER_BIT);

		glfwSwapBuffers(mainWindow);
	}

	return 0;
}