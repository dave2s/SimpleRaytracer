#pragma once
#include <GLM\glm.hpp>
class RT_Mesh
{
public:
	RT_Mesh();
	
	void CreateMesh(float *vertices, unsigned int *indices, unsigned int vertex_count, unsigned int index_count);
	void RenderMesh();
	void ClearMesh();

	static glm::vec3 getTriangleNormal(glm::vec3 vertices[3]){return glm::normalize(glm::cross(vertices[1] - vertices[0], vertices[2] - vertices[0]));}

	~RT_Mesh();

private:
	unsigned int index_count;
	float* vertices;
	unsigned int* indices;
	unsigned int vertex_count;

	void Triangulate();
};

