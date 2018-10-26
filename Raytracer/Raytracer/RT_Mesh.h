#pragma once
#include <GLM\glm.hpp>
#include "Camera.h"
#include <iostream>
#include <vector>
#define GLM_ENABLE_EXPERIMENTAL
#include "glm/gtx/string_cast.hpp"
#include "Ray.h"
class RT_Mesh
{
public:
	RT_Mesh();

	glm::u8vec4 color;
	
	void CreateMesh(const float *vertices, const unsigned int *indices, unsigned int vertices_len, unsigned int indices_len, bool singleSided, glm::u8vec4 _color);
	void ClearMesh();

	bool isSingleSided() { return singleSided; };
	int getTriangleCount() { return indices_len/3;}

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

	static glm::vec3 getTriangleNormal(std::vector<glm::vec3> vertices){ return glm::normalize(glm::cross(vertices[1] - vertices[0], vertices[2] - vertices[0])); }//triangle normal
	static glm::vec3 getTriangleUnNormal(std::vector<glm::vec3> vertices) { return glm::cross(vertices[1] - vertices[0], vertices[2] - vertices[0]); }//triangle normal not normalized
	static float getDistanceFromOrigin(glm::vec3 normal, glm::vec3 vertex) { return glm::dot(normal,vertex); }
	static float getPlaneIntersectionDistance(bool &isPrimary, float distance_from_origin, glm::vec3 plane_normal, glm::vec3 origin, glm::vec3 ray_direction, bool& _singleSided, int &e) {
		//if (!isPrimary) { std::cout << "shadow ray je singlesided? " << _singleSided << std::endl; }
		float ray_dot_normal = glm::dot(plane_normal, ray_direction); if (/*negative z camera-triangle facing away||*/ray_dot_normal == 0 || (ray_dot_normal > 0 && _singleSided)/*R.N < 0 if looking at each other..ray&plane parallel+ dont divide by zero && + if singlesided, looking otherway */) { e=1; return 0; }
		else { return (/*(-1* */(glm::dot(plane_normal, origin) + distance_from_origin)/*)*/ / /*-1* */ ray_dot_normal); }
	
	}

	static glm::vec3 getPlaneIntersection(glm::vec3 &origin, float &intersection_distance,glm::vec3 &ray_direction) { return origin + (intersection_distance*ray_direction) ;}
	///TODO
	/*static bool shadowRayHitTriangle(std::vector<glm::vec3> _triangle, Ray *ray, bool _singleSided, float& distance, glm::vec3 & PHit, float min_dist);*/
	static bool intersectTriangle(bool isPrimary,std::vector<glm::vec3> _triangle, bool _singleSided, Ray *ray,glm::vec3 &PHit, float &t, float &u, float &v, float &min_dist);
	static bool rayHitTriangle(std::vector<glm::vec3> triangle, bool isPrimary, Ray *ray, bool singleSided, float& distance, glm::vec3 & PHit, float min_dist);

	~RT_Mesh();

private:
	unsigned int indices_len;
	float* vertices;
	unsigned int* indices;
	unsigned int vertex_count;
	bool singleSided;

	///Not implemented
	void Triangulate();
	///Not implemented
	bool VerticesAreUnique(float * vertices, unsigned int vertex_count);
};

