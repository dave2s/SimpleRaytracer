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

bool RT_Mesh::raytraceTriangle(glm::vec3 triangle[3],glm::vec3 origin,glm::vec3 ray_dir, Camera camera)
{//TODO edit to use references or pointers?
	glm::vec3 n = RT_Mesh::getTriangleNormal(triangle);
	float d = getDistanceFromOrigin(n, triangle[0]);
	float hit_distance = getPlaneIntersectionDistance(d, n, origin, ray_dir);

	if (hit_distance <= NEAR_PLANE) {
		return false; // Triangle is behind the camera OR it's parallel with the ray - both invisible ///TODO too far - fake dust in air
	}

	glm::vec3 Phit = getPlaneIntersection(origin, hit_distance, ray_dir);

	//paralelni?


	return true;
}

RT_Mesh::~RT_Mesh()
{
	ClearMesh();
}
