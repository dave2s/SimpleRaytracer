#include "main.h"
#include <iostream>
//#include <stdio.h>
#include "GL/glew.h"
#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"
#include <Windows.h>
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>

extern "C" {
	_declspec(dllexport) DWORD NvOptimusEnablement = 0x00000001;
}

const GLint WIDTH = 800, HEIGHT = 600;

GLuint VAO, VBO, shader;
GLuint att_idx=0;

//Vertex Shader
static const char* vs = "\n\
#version 460								\n\
											\n\
layout (location = 0) in vec3 pos;			\n\
void main()									\n\
{											\n\
	gl_Position = vec4(pos.x,pos.y,pos.z,1.0);					\n\
}											\n\
	";

static const char* fs = "\n\
#version 460								\n\
											\n\
out vec4 color;			\n\
void main()									\n\
{											\n\
	color = vec4(1.0,0.0,0.0,1.0);					\n\
}											\n\
	";

void AddShader(GLuint shader, const char* shaderContent, GLenum shaderType) {
	GLuint _shader = glCreateShader(shaderType);
	const GLchar* _content[1];
	_content[0] = shaderContent;

	GLint contLen[1];
	contLen[0] = strlen(shaderContent);

	glShaderSource(shader, 1, _content, contLen);
	glCompileShader(shader);
	
	glAttachShader(shader, _shader);
}
void CompileShaders() { ///Prepsat na pipepline
	shader = glCreateProgram();
	if (!shader) {
		std::cerr << "Shader creation failed.";
		return;
	}

	AddShader(shader, vs, GL_VERTEX_SHADER);
	AddShader(shader, fs, GL_FRAGMENT_SHADER);

	GLint result = 0;
	GLchar log[1024] = { 0 };

	glLinkProgram(shader);
	glGetProgramiv(shader, GL_LINK_STATUS, &result);
	if(!result){
		glGetProgramInfoLog(shader, sizeof(log), NULL, log);
		std::cout << "Linking shader error: '"<<log<<"'\n";
	}

	glValidateProgram(shader);
	glGetProgramiv(shader, GL_VALIDATE_STATUS, &result);
	if (!result) {
		glGetProgramInfoLog(shader, sizeof(log), NULL, log);
		std::cout << "Valide shader error: '" << log << "'\n";
	}

}

void CreateTriangle() {
	GLfloat vertices[] = {
		-1.0f, -1.0f, 0.0f,
		1.0f, -1.0f, 0.0f,
		0.0f, 1.0f, 0.0f
	};

	glCreateVertexArrays(1, &VAO);
	glCreateVertexArrays(1, &VBO);

	glEnableVertexArrayAttrib(VAO, att_idx);
	//glEnableVertexArrayAttrib(VBO, 1);
	
	glVertexArrayVertexBuffer(VAO, 0, VBO , 0, sizeof(glm::vec4));
	
	glVertexArrayAttribBinding(VAO, att_idx, 0); //

	//glVertexArrayElementBuffer(VAO,);
	
}

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

	CreateTriangle();
	CompileShaders();

	//Window loop
	while (!glfwWindowShouldClose(mainWindow)) {
		glfwPollEvents(); // So we can read events later in the loop

		/// So we can draw next frame
		glClearColor(GLclampf(0.f), GLclampf(0.f), GLclampf(0.f), GLclampf(1.f));
		glClear(GL_COLOR_BUFFER_BIT);

		glUseProgram(shader);

		glBindVertexArray(VAO);
		glDrawArrays(GL_TRIANGLES, 0, 3);
		glBindVertexArray(0);

		glUseProgram(0);

		glfwSwapBuffers(mainWindow);
	}

	return 0;
}