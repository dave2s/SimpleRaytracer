#pragma once
#include <GLM\glm.hpp>
class RT_Mesh
{
public:
	RT_Mesh();
	
	void CreateMesh(float *vertices, unsigned int *indices, unsigned int vertex_count, unsigned int index_count);
	void IntersectPlane();
	void ClearMesh();

	static glm::vec3 getTriangleNormal(glm::vec3 vertices[3]){ return glm::normalize(glm::cross(vertices[1] - vertices[0], vertices[2] - vertices[0])); }
	static float getDistanceFromOrigin(glm::vec3 normal, glm::vec3 vertex) { return glm::dot(normal,vertex); }
	static float getPlaneIntersectionDistance(float distance_from_origin,glm::vec3 plane_normal,glm::vec3 origin, glm::vec3 ray_direction) { return ((glm::dot(plane_normal,origin)+distance_from_origin) / glm::dot(plane_normal, ray_direction)); }
	static glm::vec3 getPlaneIntersection(glm::vec3 origin, float intersection_distance,glm::vec3 ray_direction) { return origin + (intersection_distance*ray_direction) ;}

	~RT_Mesh();

private:
	unsigned int index_count;
	float* vertices;
	unsigned int* indices;
	unsigned int vertex_count;

	void Triangulate();
};

