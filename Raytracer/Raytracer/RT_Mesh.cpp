#include "RT_Mesh.h"
#include "Ray.h"
#include "Defines.h"


RT_Mesh::RT_Mesh()
{
}
/*s
void RT_Mesh::GetHitProperties()
{
}

void RT_Mesh::ClearMesh()
{
}

int RT_Mesh::getTriangleCount()
{
	return 0;
}

bool RT_Mesh::isSingleSided()
{
	return false;
}

RT_Mesh::Vertex * RT_Mesh::getTriangle(int triangle)
{
	return nullptr;
}

inline std::vector<Texture>& RT_Mesh::GetTextures()
{
	// TODO: insert return statement here
	return _textures;
}*/

RT_PolygonMesh::RT_PolygonMesh() { ; }
RT_PolygonMesh::~RT_PolygonMesh() { ; }


void RT_Mesh::GetHitProperties(RT_Mesh::Vertex& v0, Vertex& v1, Vertex& v2, float& u, float& v, int& textureHeight, int& textureWidth, glm::vec3& N, glm::vec2 &texture_coords)
{
#ifndef SMOOTH_SHADING
	N = getTriangleNormal(v0.position, v1.position, v2.position);
#else
	//Interpolate vertex normals using barycentric coordinates.
	//Perspective correction: divide by respective v.z then multiply by fragment.z
	N = glm::normalize((1 - u - v)*v0.normal + u * v1.normal + v * v2.normal);
#endif
	///Textures
	//interpolation with perspective correction (divide by respective vertex depth in camera space, then multiply by fragment depth)
	texture_coords.x = ((1 - u - v)*v0._tex_coords.x + u * v1._tex_coords.x + v * v2._tex_coords.x);
	texture_coords.y = ((1 - u - v)*v0._tex_coords.y + u * v1._tex_coords.y + v * v2._tex_coords.y);
#ifdef TEXTURE_REPEAT
	//lets repeat the texture :) /// if 1, rets 0, is ok?
	if (texture_coords.x < 0)
		texture_coords.x = 1 + texture_coords.x - (int)texture_coords.x;
	else
		texture_coords.x = texture_coords.x - (int)texture_coords.x;
	if (texture_coords.y < 0)
		texture_coords.y = 1 + texture_coords.y - (int)texture_coords.y;
	else
		texture_coords.y = texture_coords.y - (int)texture_coords.y;

#else
	texture_coords = glm::clamp(texture_coords, 0.f, 1.f);
#endif
	///Let's expand the coords from (0,1) to (0,texture_size);
	texture_coords.x = texture_coords.x*textureWidth;
	texture_coords.y = texture_coords.y*textureHeight;
}

void RT_Mesh::GetHitProperties(Vertex& v0, Vertex& v1, Vertex& v2, float u, float v, glm::vec3& N)
{
#ifndef SMOOTH_SHADING
	N = getTriangleNormal(v0.position, v1.position, v2.position);
#else
	//Interpolate vertex normals using barycentric coordinates.
	//Perspective correction: divide by respective v.z then multiply by fragment.z
	N = glm::normalize((1 - u - v)*v0.normal + u * v1.normal + v * v2.normal);
#endif
}

RT_PolygonMesh::RT_PolygonMesh(RT_PolygonMesh::Vertex* vertices, const unsigned int *indices, unsigned int vertices_len, unsigned int indices_len, bool singleSided/*, glm::f32vec3 _color*/, float albedo, RT_PolygonMesh::MATERIAL_TYPE material) {

	_indices_len = 0;
	_singleSided = false;
	glm::mat4 object_to_world;

	//color = _color;s
	_indices_len = indices_len;

	_vertices = std::vector(vertices, vertices + sizeof vertices / sizeof vertices[0]);

	for (unsigned int i = 0; i < vertices_len; ++i) {
		updateBoundaries(vertices[i]);
	}

	_indices = std::vector(indices, indices + sizeof indices / sizeof indices[0]);
	_singleSided = singleSided;
	_albedo = glm::f32vec3(albedo);
	_material_type = material;
}

RT_PolygonMesh::RT_PolygonMesh(std::vector<RT_PolygonMesh::Vertex> vertices, std::vector<unsigned int> indices, bool singleSided, Material material, float albedo, RT_PolygonMesh::MATERIAL_TYPE material_type, std::vector<Texture> textures)
	: _albedo(albedo),
	_material(material),
	_material_type(material_type),
	_textures(textures),
	_indices(indices),
	//_indices_len(indices_len),
	_vertices(vertices),
	_triangleCount(indices.size() / 3),
	_vertices_len(vertices.size()),
	_singleSided(singleSided)
{
	//_vertices = vertices;
	//_indices = indices;
	for (uint64_t i = 0; i < _vertices_len; ++i) {
		updateBoundaries(_vertices[i]);
	}
	////memcpy(indices, &_indices[0], sizeof(unsigned int)*_indices_len);
	////indices = &_indices[0];
	//_material = my_material;
	////color = material.diffuse_color;
	//_indices_len = indices_len;
	//_singleSided = singleSided;
	//_albedo = glm::f32vec3(albedo);
	//_material_type = material;
}

void RT_PolygonMesh::updateBoundaries(Vertex &vertex) {
	for (unsigned char i = 0; i < (char)vertex.position.length(); ++i) {
		if (vertex.position[i] < boundary_points[0][i]) {
			boundary_points[0][i] = vertex.position[i];
		}
		if (vertex.position[i] > boundary_points[1][i]) {
			boundary_points[1][i] = vertex.position[i];
		}
	}
}

void RT_PolygonMesh::ClearMesh()
{
}


//RT_Sphere
 RT_Sphere::RT_Sphere(glm::vec3 position, float radius, RT_Mesh::MATERIAL_TYPE type) :
	_position(position),
	_radius(radius),
	_type(type)
{
}

 void RT_Sphere::ClearMesh()
 {
 }

 int RT_Sphere::getTriangleCount()
 {
	 return 0;
 }

 bool RT_Sphere::isSingleSided()
 {
	 return false;
 }

 RT_Mesh::Vertex * RT_Sphere::getTriangle(unsigned int triangle)
 {
	 return nullptr;
 }

 inline std::vector<Texture>& RT_Sphere::GetTextures()
 {
	 // TODO: insert return statement here
	 return _textures;
 }
