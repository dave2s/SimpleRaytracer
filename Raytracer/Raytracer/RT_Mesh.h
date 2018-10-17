#pragma once
#include <GLM\glm.hpp>
#include "Camera.h"
#include <iostream>
#include <vector>
#define GLM_ENABLE_EXPERIMENTAL
#include "glm/gtx/string_cast.hpp"
class RT_Mesh
{
public:
	RT_Mesh();
	
	void CreateMesh(float *vertices, unsigned int *indices, unsigned int vertex_count, unsigned int index_count, bool singleSided);
	void ClearMesh();

	bool isSingleSided() { return singleSided; };
	int getTriangleCount() { return index_count/3;}//indices len/3

	//Return triangle by index of the triangle
	std::vector<glm::vec3> getTriangle(unsigned int idx) {
		glm::vec3 v0 = { vertices[indices[0 + 3 * idx] * 3 + 0], vertices[indices[0 + 3 * idx] * 3 + 1], vertices[indices[0 + 3 * idx] * 3 + 2] };
		glm::vec3 v1 = { vertices[indices[1 + 3 * idx] * 3 + 0], vertices[indices[1 + 3 * idx] * 3 + 1], vertices[indices[1 + 3 * idx] * 3 + 2] };
		glm::vec3 v2 = { vertices[indices[2 + 3 * idx] * 3 + 0], vertices[indices[2 + 3 * idx] * 3 + 1], vertices[indices[2 + 3 * idx] * 3 + 2] };
		std::vector<glm::vec3> triangle;
		triangle.push_back(v0);
		triangle.push_back(v1);
		triangle.push_back(v2);

		return triangle;
	}

	static glm::vec3 getTriangleNormal(std::vector<glm::vec3> vertices){ return glm::normalize(glm::cross(vertices[1] - vertices[0], vertices[2] - vertices[0])); }
	static float getDistanceFromOrigin(glm::vec3 normal, glm::vec3 vertex) { return glm::dot(normal,vertex); }
	static float getPlaneIntersectionDistance(float distance_from_origin,glm::vec3 plane_normal,glm::vec3 origin, glm::vec3 ray_direction, bool& _singleSided) {	float ray_dot_normal = glm::dot(plane_normal, ray_direction);if (ray_dot_normal == 0 || (ray_dot_normal > 0 && _singleSided)/*ray&plane parallel + dont divide by zero*/) { return 0;}	else { return ((glm::dot(plane_normal, origin) + distance_from_origin) / ray_dot_normal);}
	
	}
	static glm::vec3 getPlaneIntersection(glm::vec3 origin, float intersection_distance,glm::vec3 ray_direction) { return origin + (intersection_distance*ray_direction) ;}
	///TODO
	static bool rayHitTriangle(std::vector<glm::vec3> triangle, glm::vec3 origin, glm::vec3 ray_dir, Camera camera, bool singleSided);

	~RT_Mesh();

private:
	unsigned int index_count;
	float* vertices;
	unsigned int* indices;
	unsigned int vertex_count;
	bool singleSided;

	///Not implemented
	void Triangulate();
	///Not implemented
	bool VerticesAreUnique(float * vertices, unsigned int vertex_count);
};

