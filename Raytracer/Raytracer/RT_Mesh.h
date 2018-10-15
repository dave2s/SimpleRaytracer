#pragma once
#include <GLM\glm.hpp>
#include "Camera.h"
class RT_Mesh
{
public:
	RT_Mesh();
	
	void CreateMesh(float *vertices, unsigned int *indices, unsigned int vertex_count, unsigned int index_count);
	void ClearMesh();

	int getTriangleCount() { return (sizeof(indices)/sizeof(*indices)) / 3; }
	//Return triangle by index of the triangle
	glm::vec3* getTriangle(unsigned int idx) {
		glm::vec3 v0 = { vertices[indices[idx * 9 + 0]], vertices[indices[idx * 9 + 1]], vertices[indices[idx * 9 + 2]]};
		glm::vec3 v1 = { vertices[indices[idx * 9 + 3]], vertices[indices[idx * 9 + 4]], vertices[indices[idx * 9 + 6]] };
		glm::vec3 v2 = { vertices[indices[idx * 9 + 6]], vertices[indices[idx * 9 + 5]], vertices[indices[idx * 9 + 7]] };
		glm::vec3 triangle[3] = { v0,v1,v2 };
		return triangle;
	}

	static glm::vec3 getTriangleNormal(glm::vec3 vertices[3]){ return glm::normalize(glm::cross(vertices[1] - vertices[0], vertices[2] - vertices[0])); }
	static float getDistanceFromOrigin(glm::vec3 normal, glm::vec3 vertex) { return glm::dot(normal,vertex); }
	static float getPlaneIntersectionDistance(float distance_from_origin,glm::vec3 plane_normal,glm::vec3 origin, glm::vec3 ray_direction) {float ray_dot_normal = glm::dot(plane_normal, ray_direction);if (ray_dot_normal == 0/*ray&plane parallel + dont divide by zero*/) { return 0;}	else { return ((glm::dot(plane_normal, origin) + distance_from_origin) / ray_dot_normal);}
	
	}
	static glm::vec3 getPlaneIntersection(glm::vec3 origin, float intersection_distance,glm::vec3 ray_direction) { return origin + (intersection_distance*ray_direction) ;}
	///TODO
	static bool rayHitTriangle(glm::vec3 triangle[3], glm::vec3 origin, glm::vec3 ray_dir, Camera camera);

	~RT_Mesh();

private:
	unsigned int index_count;
	float* vertices;
	unsigned int* indices;
	unsigned int vertex_count;

	void Triangulate();
};

