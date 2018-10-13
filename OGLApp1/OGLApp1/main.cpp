#include "main.h"
#include <iostream>
#include <string.h>
#include <vector>
#include "GL/glew.h"
//#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"
#include <Windows.h>
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "AppWindow.h"
#include "Mesh.h"
#include "Shader.h"


extern "C" {
	_declspec(dllexport) DWORD NvOptimusEnablement = 0x00000001;
}

#define NEAR_PLANE 0.1f
#define FAR_PLANE 100.0f

const float toRadians = glm::pi<float>() / 180.0f;

glm::vec3 cam_pos, cam_dir, cam_up;

AppWindow main_window;
std::vector<Mesh*> mesh_vec;
std::vector<Shader> shader_list;

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
/*
*	I need to calculate the view matrix by setting
*	the position for camera
*	a direction calculated from pitch, yaw and possibly roll
*	the world upwards position
*	dir - relative direction to position
*	pos - absolute position of camera in the world
*	up - absolute world up vector
*/

int main()
{
	main_window = AppWindow(800, 600);
	main_window.Init();

	CreateObj();
	CreateShaders();

	GLuint uniform_projection = 0, uniform_model = 0;
	///Declare mvp matrices
	glm::mat4 projection_mat, view_mat, model_mat;
	///Create projection
	projection_mat = glm::perspective(45.0f, (GLfloat)main_window.getBufferWidth() / (GLfloat)main_window.getBufferHeight(), NEAR_PLANE, FAR_PLANE);
	projection_mat = glm::translate(projection_mat, glm::vec3(0.0f, 0.0f, -2.0f));

	//Window loop
	while (!main_window.shouldClose()) {
		glfwPollEvents(); // So we can read events later in the loop

		/// So we can draw next frame
		glClearColor(GLclampf(0.f), GLclampf(0.f), GLclampf(0.f), GLclampf(1.f));
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		shader_list[0].UseShader();
		uniform_model = shader_list[0].GetLocationModel();
		uniform_projection = shader_list[0].GetLocationProjection();
		//uniform_view = shader_list[0].GetLocationView();

		model_mat = glm::mat4(1.0f);
		model_mat = glm::translate(model_mat, glm::vec3(0.0f, 0.0f, 0.0f));
		model_mat = glm::scale(model_mat, glm::vec3(0.4f, 0.4f, 1.0f));

		//view_mat = glm::lookAt(cam_pos, cam_dir, cam_up);

		glUniformMatrix4fv(uniform_model,1,GL_FALSE,glm::value_ptr(model_mat));
		mesh_vec[0]->RenderMesh();

		model_mat = glm::mat4(1.0f);
		model_mat = glm::translate(model_mat, glm::vec3(0.0f, 1.0f, 0.0f));
		model_mat = glm::scale(model_mat, glm::vec3(0.4f, 0.4f, 1.0f));
		glUniformMatrix4fv(uniform_model, 1, GL_FALSE, glm::value_ptr(model_mat));

		mesh_vec[1]->RenderMesh();

		glUniformMatrix4fv(uniform_projection, 1, GL_FALSE, glm::value_ptr(projection_mat));
		glUseProgram(0);

		main_window.SwapBuffers();
	}

	return 0;
}