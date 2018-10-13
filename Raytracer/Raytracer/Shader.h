#pragma once

#include <iostream>
#include<fstream>
#include<string>
#include<stdio.h>

#include <GL\glew.h>

class Shader
{
public:
	Shader();
	~Shader();

	std::string ReadShaderFile(const char* file_path);
	void CreateShader(const char * vs_path, const char * fs_path);

	GLuint GetLocationProjection();
	GLuint GetLocationModel();
	GLuint GetLocationView();

	void UseShader();
	void ClearShader();

private:
	GLuint shader_id, uniform_projection, uniform_view, uniform_model;

	void CompileShader(const char * code_vs, const char * code_fs);
	void AddShader(GLuint program, const char* content, GLenum type);
};

