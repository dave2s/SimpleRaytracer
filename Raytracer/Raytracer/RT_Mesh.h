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

	enum MATERIAL_TYPE {DIFFUSE, REFLECTIVE, MIRROR};

	struct Vertex {
		glm::f32vec3 position;
		glm::f32vec3 normal;
		glm::f32vec2 tex_coords;
	};
	struct Texture {
		unsigned int id;
		std::string type;
		std::string path;
	};

	std::vector<Texture> textures;

	glm::f32vec3 color;
	glm::f32vec3 albedo;
	MATERIAL_TYPE material_type;

	RT_Mesh(Vertex* vertices, const unsigned int *indices, unsigned int vertices_len, unsigned int indices_len, bool singleSided, glm::f32vec3 _color, float albedo, MATERIAL_TYPE material);
	void ClearMesh();

	bool isSingleSided() { return singleSided; };
	int getTriangleCount() { return indices_len/3;}

	//Return triangle by index of the triangle
	Vertex* getTriangle(unsigned int idx) {
		/*glm::vec3 v0 = { vertices[indices[0 + 3 * idx] * 3 + 0], vertices[indices[0 + 3 * idx] * 3 + 1], vertices[indices[0 + 3 * idx] * 3 + 2] };
		glm::vec3 v1 = { vertices[indices[1 + 3 * idx] * 3 + 0], vertices[indices[1 + 3 * idx] * 3 + 1], vertices[indices[1 + 3 * idx] * 3 + 2] };
		glm::vec3 v2 = { vertices[indices[2 + 3 * idx] * 3 + 0], vertices[indices[2 + 3 * idx] * 3 + 1], vertices[indices[2 + 3 * idx] * 3 + 2] };*/
		//glm::f32vec3 v0 = vertices[0 + 3 * idx].position;
		//glm::f32vec3 v1 = vertices[1 + 3 * idx].position;
		//glm::f32vec3 v2 = vertices[2 + 3 * idx].position;
		/*std::vector<glm::vec3> triangle;
		triangle.push_back(v0);
		triangle.push_back(v1);
		triangle.push_back(v2);*/
		Vertex triangle[3] = { vertices[indices[0 + 3 * idx]],vertices[indices[1 + 3 * idx]], vertices[indices[2 + 3 * idx]] };
		return triangle;
	}

	static glm::vec3 getTriangleNormal(glm::vec3* vertices){ return glm::normalize(glm::cross(vertices[1] - vertices[0], vertices[2] - vertices[0])); }//triangle normal
	static glm::vec3 getTriangleUnNormal(glm::vec3* vertices) { return glm::cross(vertices[1] - vertices[0], vertices[2] - vertices[0]); }//triangle normal not normalized
	static float getDistanceFromOrigin(glm::vec3 normal, glm::vec3 vertex) { return glm::dot(normal,vertex); }
	static float getPlaneIntersectionDistance(bool &isPrimary, float distance_from_origin, glm::vec3 plane_normal, glm::vec3 origin, glm::vec3 ray_direction, bool& _singleSided, int &e) {
		//if (!isPrimary) { std::cout << "shadow ray je singlesided? " << _singleSided << std::endl; }
		float ray_dot_normal = glm::dot(plane_normal, ray_direction); if (/*negative z camera-triangle facing away||*/ray_dot_normal == 0 || (ray_dot_normal > 0 && _singleSided)/*R.N < 0 if looking at each other..ray&plane parallel+ dont divide by zero && + if singlesided, looking otherway */) { e=1; return 0; }
		else { return (/*(-1* */(glm::dot(plane_normal, origin) + distance_from_origin)/*)*/ / /*-1* */ ray_dot_normal); }
	
	}

	static glm::vec3 getPlaneIntersection(glm::vec3 &origin, float &intersection_distance,glm::vec3 &ray_direction) { return origin + (intersection_distance*ray_direction) ;}
	///TODO
	/*static bool shadowRayHitTriangle(std::vector<glm::vec3> _triangle, Ray *ray, bool _singleSided, float& distance, glm::vec3 & PHit, float min_dist);*/
	static bool intersectTriangle(bool isPrimary,glm::vec3* _triangle, bool _singleSided, Ray *ray,glm::vec3 &PHit, float &t, float &u, float &v, float &min_dist);
	static bool intersectTriangleMT(bool isPrimary, Vertex* _triangle, bool _singleSided, Ray *ray, glm::vec3 &PHit, glm::vec3 & NHit, float &t, float &u, float &v, float &min_dist);
	static bool rayHitTriangle(glm::vec3* triangle, bool isPrimary, Ray *ray, bool singleSided, float& distance, glm::vec3 & PHit, float min_dist);

	~RT_Mesh();

private:
	unsigned int indices_len;
	unsigned int* indices;
	Vertex* vertices;
	unsigned int vertex_count;
	bool singleSided;
	
	///Not implemented
	void Triangulate();
	///Not implemented
	bool VerticesAreUnique(float * vertices, unsigned int vertex_count);
};

