#include "main.h"
#include <iostream>
#include <string.h>
//#include <stdio.h>
#include "GL/glew.h"
//#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"
#include <Windows.h>
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
//#include <glm/vec4.hpp>
//#include <glm/mat4x4.hpp>

extern "C" {
	_declspec(dllexport) DWORD NvOptimusEnablement = 0x00000001;
}

const GLint WIDTH = 800, HEIGHT = 600;
const float toRadians = 3.14159265f/180.0f;

GLuint VAO, VBO, shader, uniformModel;
GLuint att_idx = 0;

bool direction = true;
float triOffset = 0.0f;
float triMaxoffset = 0.7f;
float triIncrement = 0.0005f;

float curAngle = 0.0f;

//Vertex Shader
static const char* vs = "#version 460 \n\
layout(location = 0) in vec3 pos; \n\
uniform mat4 model;	\n\
 void main() { \n\ gl_Position = model * vec4(pos, 1.0); \n\ }";
//Fragment Shader
static const char* fs = "#version 460\n\
out vec4 color;\n\
void main(){\n\
	color = vec4(0.0,1.0,0.0,1.0);\n\
}";

void AddShader(GLuint program, const char* content, GLenum type) {
	GLuint shader = glCreateShader(type);
	const GLchar* _content[1];
	_content[0] = content;
	GLint contentLen[1];
	contentLen[0] = GLint(strlen(content));

	glShaderSource(shader, 1, _content, contentLen);
	glCompileShader(shader);

	GLint result = 0;
	GLchar eLog[1024] = { 0 };

	glGetShaderiv(shader, GL_COMPILE_STATUS, &result);
	if (!result) {
		glGetShaderInfoLog(shader, sizeof(eLog), NULL, eLog);
		std::cerr << "Shader compile failed:" << eLog << "\n";
		return;
	}
	glAttachShader(program, shader);
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
	if (!result) {
		glGetProgramInfoLog(shader, sizeof(log), NULL, log);
		std::cout << "Linking shader error: '" << log << "'\n";
	}

	glValidateProgram(shader);
	glGetProgramiv(shader, GL_VALIDATE_STATUS, &result);
	if (!result) {
		glGetProgramInfoLog(shader, sizeof(log), NULL, log);
		std::cout << "Valide shader error: '" << log << "'\n";
	}
	uniformModel = glGetUniformLocation(shader, "model");
}

void CreateTriangle() {
	GLfloat vertices[] = {
		-1.0f, -1.0f, 0.0f,
		1.0f, -1.0f, 0.0f,
		0.0f, 1.0f, 0.0f
	};

	//glGenVertexArrays(1, &VAO);
	glCreateVertexArrays(1, &VAO);
	glBindVertexArray(VAO);

	//glGenBuffers(1, &VBO);
	glCreateBuffers(1, &VBO);
	glNamedBufferData(VBO,sizeof(vertices),vertices,GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	//glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexArrayAttrib(VAO,0);
	//glVertexArrayAttribFormat(VAO, 0, 3, GL_FLOAT, GL_FALSE, 0);

	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glBindVertexArray(0);

}
/*
void AtomicCounterExample() {
	unsigned data[4] = { 0,0,0,0 };
	GLuint ACB;
	float Max = 256;
	glCreateBuffers(1, &ACB); //3
	glNamedBufferData(ACB, sizeof(uint32_t) * 4, data, GL_DYNAMIC_DRAW);
	glBindBufferBase(GL_ATOMIC_COUNTER_BUFFER, 1, ACB); //binding point 1

	GLuint SSBO; //identifier of shader storage buffer
	glCreateBuffers(1, &SSBO);
	glNamedBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(float) * 4 * 2 * Max, NULL, GL_DYNAMIC_DRAW);
	glClearNamedBufferData(SSBO, GL_R32F, GL_RED, GL_FLOAT, NULL);
	glBindBufferRange(GL_SHADER_STORAGE_BUFFER, 0, SSBO, 0, sizeof(float) * 4 * 2 * Max); //binding 0
}*/

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
	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);

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
	//glMatrixMode(GL_PROJECTION);
	CreateTriangle();
	//AtomicCounterExample();
	CompileShaders();

	//Window loop
	while (!glfwWindowShouldClose(mainWindow)) {
		glfwPollEvents(); // So we can read events later in the loop

		if (direction) {
			triOffset += triIncrement;
		}
		else {
			triOffset -= triIncrement;
		}		
		if (abs(triOffset) >= triMaxoffset) {
			direction = !direction;
		}

		curAngle += 0.1f;
		if (curAngle >= 360) {
			curAngle -= 360;
		}

		/// So we can draw next frame
		glClearColor(GLclampf(0.f), GLclampf(0.f), GLclampf(0.f), GLclampf(1.f));
		glClear(GL_COLOR_BUFFER_BIT);

		glUseProgram(shader);

		glm::mat4 model = glm::mat4(1.0f);
		glm::mat4 view = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(triOffset, 0.0f, 0.0f));
		model = glm::rotate(model, curAngle * toRadians, glm::vec3(0.0f, 0.0f, 1.0f));
		model = glm::scale(model, glm::vec3(0.4f, 0.4f, 1.0f));

		view = glm::translate(view, glm::vec3(cameraX, cameraY, 0.0f));

		glUniformMatrix4fv(uniformModel,1,GL_FALSE,glm::value_ptr(model));

		glBindVertexArray(VAO);
		glDrawArrays(GL_TRIANGLES, 0, 3);
		glBindVertexArray(0);

		glUseProgram(0);

		glfwSwapBuffers(mainWindow);
	}

	return 0;
}