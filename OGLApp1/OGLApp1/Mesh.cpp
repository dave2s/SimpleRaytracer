#include "Mesh.h"



Mesh::Mesh()
{
	VAO = VBO = IBO = _index_count = 0;
}

void Mesh::CreateMesh(GLfloat *vertices, GLuint *indices, unsigned int vertex_count, unsigned int index_count) {
	_index_count = index_count;

	//glGenVertexArrays(1, &VAO);
	glCreateVertexArrays(1, &VAO);
	glBindVertexArray(VAO);

	glCreateBuffers(1, &IBO);
	glNamedBufferData(IBO, sizeof(indices[0]) * index_count, indices, GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);

	//glGenBuffers(1, &VBO);
	glCreateBuffers(1, &VBO);
	glNamedBufferData(VBO, sizeof(vertices[0]) * vertex_count, vertices, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	//glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexArrayAttrib(VAO, 0);
	//glVertexArrayAttribFormat(VAO, 0, 3, GL_FLOAT, GL_FALSE, 0);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	glBindVertexArray(0);
}

Mesh::~Mesh()
{
}
