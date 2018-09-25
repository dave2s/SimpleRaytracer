// OGLApp1.cpp : This file contains the 'main' function. Program execution begins and ends there.

#include "pch.h"
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
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3); //X
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3); //Y
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

	if (glewInit() != GLEW_OK){ 
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

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
