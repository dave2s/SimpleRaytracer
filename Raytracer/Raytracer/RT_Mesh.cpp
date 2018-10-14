#include "RT_Mesh.h"



RT_Mesh::RT_Mesh()
{
	index_count = 0;
}

void RT_Mesh::CreateMesh(float *_vertices, unsigned int *_indices, unsigned int _vertex_count, unsigned int _index_count) {
	index_count = _index_count;
	vertices = _vertices;
	vertex_count = _vertex_count;
	indices = _indices;
}

void RT_Mesh::IntersectPlane()
{

}

void RT_Mesh::ClearMesh()
{

}

RT_Mesh::~RT_Mesh()
{
	ClearMesh();
}
