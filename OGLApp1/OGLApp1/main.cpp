#include "main.h"
#include <iostream>
#include <string.h>
#include <vector>
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
#include "AppWindow.h"
#include "Mesh.h"
#include "Shader.h"


extern "C" {
	_declspec(dllexport) DWORD NvOptimusEnablement = 0x00000001;
}

#define NEAR_PLANE 0.1f
#define FAR_PLANE 100.0f

const GLint WIDTH = 800, HEIGHT = 600;
const float toRadians = 3.14159265f/180.0f;

AppWindow main_window;
std::vector<Mesh*> mesh_vec;
std::vector<Shader> shader_list;

bool direction = true;
float triOffset = 0.0f;
float triMaxoffset = 0.7f;
float triIncrement = 0.005f;

float curAngle = 0.0f;

//Vertex Shader
static const char* vs = "Shaders/shader.vs";
//Fragment Shader
static const char* fs = "Shaders/shader.fs";

void CreateObj() {
	GLfloat vertices[] = {
		-1.0f, -1.0f, 0.0f,
		0.0f, -1.0f, 1.0f,
		1.0f, -1.0f, 0.0f,
		0.0f, 1.0f, 0.0f
	};

	GLuint indices[] = {
		0, 3, 1,
		1, 3, 2,
		2, 3, 0,
		0, 1, 2
	};
	
	Mesh* mesh = new Mesh();
	mesh->CreateMesh(vertices, indices, (sizeof(vertices) / sizeof(*vertices)), (sizeof(indices) / sizeof(*indices)));
	mesh_vec.push_back(mesh);

	Mesh* mesh2 = new Mesh();
	mesh2->CreateMesh(vertices, indices, (sizeof(vertices) / sizeof(*vertices)), (sizeof(indices) / sizeof(*indices)));
	mesh_vec.push_back(mesh2);
}

void CreateShaders()
{
	Shader* shader1 = new Shader();
	shader1->CreateShader(vs, fs);
	shader_list.push_back(*shader1);	
}

int main()
{
	main_window = AppWindow(800, 600);
	main_window.initialise();

	CreateObj();
	CreateShaders();

	GLuint uniform_projection = 0, uniform_model = 0;

	///Create projection
	glm::mat4 projection_mat = glm::perspective(45.0f, main_window.getBufferWidth() / main_window.getBufferHeight(), NEAR_PLANE, FAR_PLANE);
	projection_mat = glm::translate(projection_mat, glm::vec3(0.0f, 0.0f, -2.0f));

	//Window loop
	while (!glfwWindowShouldClose(main_window)) {
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
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		shader_list[0].UseShader();
		uniform_model = shader_list[0].GetLocationModel();
		uniform_projection = shader_list[0].GetLocationProjection();

		glm::mat4 model_mat = glm::mat4(1.0f);
		glm::mat4 view = glm::mat4(1.0f);
		model_mat = glm::translate(model_mat, glm::vec3(0.0f, triOffset, 0.0f));
		model_mat = glm::scale(model_mat, glm::vec3(0.4f, 0.4f, 1.0f));

		glUniformMatrix4fv(uniform_model,1,GL_FALSE,glm::value_ptr(model_mat));
		mesh_vec[0]->RenderMesh();

		model_mat = glm::mat4(1.0f);
		model_mat = glm::translate(model_mat, glm::vec3(0.75f, -triOffset, 0.0f));
		model_mat = glm::scale(model_mat, glm::vec3(0.4f, 0.4f, 1.0f));
		glUniformMatrix4fv(uniform_model, 1, GL_FALSE, glm::value_ptr(model_mat));

		mesh_vec[1]->RenderMesh();

		glUniformMatrix4fv(uniform_projection, 1, GL_FALSE, glm::value_ptr(projection_mat));
		glUseProgram(0);

		glfwSwapBuffers(main_window);
	}

	return 0;
}