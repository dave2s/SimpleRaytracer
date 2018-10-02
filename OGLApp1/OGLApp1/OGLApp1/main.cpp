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

static const char* vs = "#version 460 \n\ layout (location = 0) in vec3 pos; \n\ void main(){\n\gl_Position = vec4(0.4*pos.x,0.4*pos.y,0.4*pos.z,1.0);\n\}";
static const char* fs = "#version 460 \n\ out vec4 color; \n\ void main(){\n\color = vec4(0.0,1.0,0.0,1.0);\n\}";

void CreateTriangle() {
	GLfloat vertices[] = {
		-1.0f, -1.0f, 0.0f,
		1.0f, -1.0f, 0.0f,
		0.0f, 1.0f, 0.0f
	};
	GLint pos_att = 0;
	GLint pos_att_num = 3;

	//glGenVertexArrays(1, &VAO);
	//glBindVertexArray(VAO);
	glCreateVertexArrays(1, &VAO);	
	glCreateBuffers(1, &VBO);
	//glGenBuffers(1, &VBO);
	//glBindBuffer(GL_ARRAY_BUFFER, VBO);
	
	glEnableVertexArrayAttrib(VAO, pos_att);
	glVertexArrayAttribFormat(VAO, pos_att, pos_att_num, GL_FLOAT, GL_FALSE, 0);
	glVertexArrayAttribBinding(VAO, pos_att, 0);

	glVertexArrayVertexBuffer(VAO, 0, VBO, 0, 0);

	//glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	//glEnableVertexAttribArray(0);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	glBindVertexArray(0);
}

void AddShader(GLuint program, const char* content, GLenum type) {
	GLuint shader = glCreateShader(type);
	const GLchar* _content[1];
	_content[0] = content;
	GLint contentLen[1];
	contentLen[0] = strlen(content);

	glShaderSource(shader, 1, _content, contentLen);
	glCompileShader(shader);

	GLint result = 0;
	GLchar eLog[1024] = { 0 };
	
	glGetShaderiv(shader, GL_COMPILE_STATUS, &result);
	if (!result) {
		glGetShaderInfoLog(shader, sizeof(eLog), NULL, eLog);
		std::cerr << "Shader compile failed:"<<eLog<<"\n";
		return;
	}
	glAttachShader(program, shader);
}

void CompileShaders() {
	shader = glCreateProgram();
	if (!shader) {
		std::cerr << "glCreateProgram() failed \n";
	}

	AddShader(shader, vs, GL_VERTEX_SHADER);
	AddShader(shader, fs, GL_FRAGMENT_SHADER);

	GLint result = 0;
	GLchar eLog[1024] = { 0 };

	glLinkProgram(shader);
	glGetProgramiv(shader, GL_LINK_STATUS, &result);

	if (!result) {
		glGetProgramInfoLog(shader, sizeof(eLog), NULL, eLog);
		std::cerr << "Error linking program:"<< eLog <<"\n";
		//printf("Error linking program: %s \n", eLog);
		return;
	}
	glValidateProgram(shader);
	glGetProgramiv(shader, GL_VALIDATE_STATUS, &result);
	if (!result) {
		glGetProgramInfoLog(shader, sizeof(eLog), NULL, eLog);
		std::cerr << "Error validating program:" << eLog << "\n";
		return;
	}
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