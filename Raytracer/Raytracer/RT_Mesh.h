#pragma once
#include <GLM\glm.hpp>
#include "Camera.h"
#include <iostream>
#include <vector>
#include "Ray.h"
#define GLM_ENABLE_EXPERIMENTAL
#include "glm/gtx/string_cast.hpp"

struct Vertex {
	glm::f32vec3 position;
	glm::f32vec3 normal;
	glm::f32vec2 _tex_coords;
};

struct Texture {
	unsigned char* data;
	uint32_t id;
	std::string type;
	std::string path;
	int width;
	int height;
	int channels;
};

struct Material {
	glm::f32vec3 diffuse_color;
	glm::f32vec3 specluar_color;
	glm::f32vec3 ambient_color;
	glm::f32vec3 emissive_color;
	float ior;
	float shininess;
};

inline glm::vec3 getTriangleNormal(glm::vec3& v0, glm::vec3& v1, glm::vec3& v2)
{
	return glm::normalize(glm::cross(v1 - v0, v2 - v0));
}
inline glm::vec3 getTriangleUnNormal(glm::vec3& v0, glm::vec3& v1, glm::vec3& v2)
{
	return glm::cross(v1 - v0, v2 - v0);
}

void GetHitProperties(Vertex& v0, Vertex& v1, Vertex& v2, float& u, float& v, int& textureHeight, int& textureWidth, glm::vec3& N, glm::vec2 &texture_coords);

void GetHitProperties(Vertex& v0, Vertex& v1, Vertex& v2, float u, float v, glm::vec3& N);

static bool intersectTriangleMT(Ray* ray, bool isPrimary, Vertex& v0, Vertex& v1, Vertex& v2, bool _singleSided, glm::vec3 &PHit, glm::vec3 & NHit, float &t, float &u, float &v, float min_dist);

class RT_Mesh
{
public:
	RT_Mesh();

	enum MATERIAL_TYPE : int {DIFFUSE, REFLECTIVE, MIRROR, PHONG, REFRACTION};

	///Two points furthest apart to form a axis aligned bouning box
	glm::vec3 boundary_points[2] = { glm::vec3(inf), glm::vec3(-inf) };
	
	Material _material;
	glm::f32vec3 _albedo;
	MATERIAL_TYPE _material_type;
	std::vector<Vertex> _vertices;
	std::vector<unsigned int> _indices;

	RT_Mesh(std::vector<Vertex> vertices, std::vector< unsigned int> indices, bool singleSided, Material my_material, float albedo, MATERIAL_TYPE material, std::vector<Texture> textures);
	void ClearMesh();

	inline bool isSingleSided() { return _singleSided; }
	inline uint32_t getTriangleCount() const { return _triangle_count; }
	inline const std::vector<Texture>& GetTextures() const
	{
		return _textures;
	}

	void computeBounds(const glm::f32vec3 &planeNormal, float &dnear, float &dfar) const;

	bool intersect_triangle(Ray * ray, uint32_t triangle, float & t_near, Ray::Hitinfo & info) const;

    void updateBoundaries(Vertex &vertex);

	//Return triangle by index of the triangle
	Vertex* getTriangle(unsigned int idx)const {
		Vertex triangle[3] = { _vertices[_indices[0 + 3 * idx]],_vertices[_indices[1 + 3 * idx]], _vertices[_indices[2 + 3 * idx]] };
		return triangle;
	}

	//triangle normal not normalized
	static float getDistanceFromOrigin(glm::vec3 normal, glm::vec3 vertex) { return glm::dot(normal,vertex); }
	static float getPlaneIntersectionDistance(bool &isPrimary, float distance_from_origin, glm::vec3 plane_normal, glm::vec3 origin, glm::vec3 ray_direction, bool& _singleSided, int &e) {
		//if (!isPrimary) { std::cout << "shadow ray je singlesided? " << _singleSided << std::endl; }
		float ray_dot_normal = glm::dot(plane_normal, ray_direction); if (/*negative z camera-triangle facing away||*/ray_dot_normal == 0 || (ray_dot_normal > 0 && _singleSided)/*R.N < 0 if looking at each other..ray&plane parallel+ dont divide by zero && + if singlesided, looking otherway */) { e=1; return 0; }
		else { return (/*(-1* */(glm::dot(plane_normal, origin) + distance_from_origin)/*)*/ / /*-1* */ ray_dot_normal); }
	
	}
	static glm::vec3 getPlaneIntersection(glm::vec3 &origin, float &intersection_distance,glm::vec3 &ray_direction) { return origin + (intersection_distance*ray_direction) ;}

	bool intersect(Ray* ray,float& a, Ray::Hitinfo& info) const;

	~RT_Mesh();

private:
	unsigned int _indices_len;

	
	unsigned int _vertices_len;
	bool _singleSided;
	uint32_t _triangle_count;
	 std::vector<Texture> _textures;
	
	///Not implemented
	void Triangulate();
	///Not implemented
	bool VerticesAreUnique(float * vertices, unsigned int vertex_count);
};