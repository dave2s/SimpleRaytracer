// OGLApp1.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "pch.h"
#include <iostream>
#include <stdio.h>
#include "GL/glew.h"
#include "GLFW/glfw3.h"

const GLint WIDTH = 1600, HEIGHT = 900;

int main()
{
	/// Initialise GLFW (windows)
	if (!glfwInit()) { 
		std::cout << "GLFW init failed!";
		glfwTerminate();
		return 1; //TODO ERRORS
	}

	/// Setup GLFW window properties
	// OpenGL version X.Y
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3); //X
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3); //Y
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    //std::cout << "Hello World!\n"; 
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
