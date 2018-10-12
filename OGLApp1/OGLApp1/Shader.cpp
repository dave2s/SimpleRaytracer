#include "Shader.h"



Shader::Shader()
{
	shader_id = uniform_projection = uniform_model = 0;
}

GLuint Shader::GetLocationProjection(){	return uniform_projection; }
GLuint Shader::GetLocationModel(){ return uniform_model; }

std::string Shader::ReadShaderFile(const char* file_path) {
	std::string code = "";
	std::string line;

	std::ifstream fileStream(file_path, std::ios::in);

	if(!fileStream.is_open()) {
		std::cerr << "could not open given file path: " << file_path;
		return code;
	}
	
	line = "";
	while (!fileStream.eof()) {		
		std::getline(fileStream, line);
		code.append(line + "\n");
		std::cout << code << "\n";
	}
	fileStream.close();

	return code;
}

void Shader::CreateShader(const char * vs_path, const char * fs_path)
{
	const char * code_vs = ReadShaderFile(vs_path).c_str();
	const char * code_fs = ReadShaderFile(fs_path).c_str();
	CompileShader(code_vs, code_fs);
}

void Shader::CompileShader(const char * code_vs,const char * code_fs)
{
	shader_id = glCreateProgram();
	if (!shader_id) {
		std::cerr << "Shader creation failed.";
		return;
	}

	AddShader(shader_id, code_vs, GL_VERTEX_SHADER);
	AddShader(shader_id, code_fs, GL_FRAGMENT_SHADER);

	GLint result = 0;
	GLchar log[1024] = { 0 };

	glLinkProgram(shader_id);
	glGetProgramiv(shader_id, GL_LINK_STATUS, &result);
	if (!result) {
		glGetProgramInfoLog(shader_id, sizeof(log), NULL, log);
		std::cout << "Linking shader error: '" << log << "'\n";
	}

	glValidateProgram(shader_id);
	glGetProgramiv(shader_id, GL_VALIDATE_STATUS, &result);
	if (!result) {
		glGetProgramInfoLog(shader_id, sizeof(log), NULL, log);
		std::cout << "Valide shader error: '" << log << "'\n";
	}
	uniform_model = glGetUniformLocation(shader_id, "model");
	uniform_projection = glGetUniformLocation(shader_id, "projection");
}

void Shader::AddShader(GLuint program, const char * content, GLenum type)
{
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

void Shader::UseShader() 
{
	glUseProgram(shader_id);
}

void Shader::ClearShader()
{
	if (shader_id != 0) {
		glDeleteProgram(shader_id);
		shader_id = 0;
	}
	uniform_model = uniform_projection = 0;
}

Shader::~Shader()
{
	ClearShader();
}
