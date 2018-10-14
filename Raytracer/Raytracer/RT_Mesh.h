#pragma once
#include <GL\glew.h>
class Mesh
{
public:
	Mesh();
	
	void CreateMesh(GLfloat *vertices, GLuint *indices, unsigned int vertex_count, unsigned int index_count);
	void RenderMesh();
	void ClearMesh();

	~Mesh();

private:
	GLuint VAO, VBO, IBO;
	GLsizei _index_count;
};

