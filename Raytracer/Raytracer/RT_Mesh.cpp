#include "RT_Mesh.h"



RT_Mesh::RT_Mesh()
{
	index_count = 0;
	singleSided = false;
}

void RT_Mesh::CreateMesh(float *_vertices, unsigned int *_indices, unsigned int _vertex_count, unsigned int _index_count, bool _singleSided) {
	index_count = _index_count;
	vertices = _vertices;
	vertex_count = _vertex_count;
	indices = _indices;
	singleSided = _singleSided;
}

void RT_Mesh::ClearMesh()
{

}

bool RT_Mesh::rayHitTriangle(glm::vec3 triangle[3],glm::vec3 origin,glm::vec3 ray_dir, Camera camera, bool& _singleSided)
{//TODO edit to use references or pointers?
	//std::cout << "kreslim vertexy: v0: " << glm::to_string(triangle[0]) << " a v1: " << glm::to_string(triangle[1]) << " a v2: " << glm::to_string(triangle[2]) << " \n";
	glm::vec3 normal = RT_Mesh::getTriangleNormal(triangle);//is normalized

	float d = getDistanceFromOrigin(normal, triangle[0]);
	float hit_distance = getPlaneIntersectionDistance(d, normal, origin, ray_dir, _singleSided);

	if (hit_distance <= CAM_NEAR_PLANE/*&&hit_distance==0*/) {
		return false; // Triangle is behind the camera OR it's parallel with the ray OR it's faced the other way and is single sided - both invisible ///TODO too far - fake dust in air
	}
	glm::vec3 Phit = getPlaneIntersection(origin, hit_distance, ray_dir);

	/*if (!(glm::dot(normal, glm::cross(triangle[1] - triangle[0], Phit - triangle[0] )) > 0 &&
		glm::dot(normal, glm::cross(triangle[2] - triangle[1], Phit - triangle[1])) > 0 &&
		glm::dot(normal, glm::cross(triangle[0] - triangle[2], Phit - triangle[2])) > 0
		));*/
	//TODO encapsulate
	///is Phit inside the triangle? test against each edge
	if (glm::dot(normal, glm::cross(triangle[1] - triangle[0], Phit - triangle[0])) < 0) { return false; }
	if (glm::dot(normal, glm::cross(triangle[2] - triangle[1], Phit - triangle[1])) < 0) { return false; }
	if (glm::dot(normal, glm::cross(triangle[0] - triangle[2], Phit - triangle[2])) < 0) { return false; }


	return true; //successful hit
}

RT_Mesh::~RT_Mesh()
{
	ClearMesh();
}
