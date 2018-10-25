#include "RT_Mesh.h"

RT_Mesh::RT_Mesh()
{
	indices_len = 0;
	singleSided = false;
}

void RT_Mesh::CreateMesh(const float *_vertices, const unsigned int *_indices, unsigned int _vertices_len, unsigned int _indices_len, bool _singleSided, glm::u8vec4 _color) {
	color = _color;
	indices_len = _indices_len;
	vertices = new float[_vertices_len];
	memcpy(vertices,_vertices,sizeof(float)*_vertices_len);	
	vertex_count = _vertices_len;
	indices = new unsigned int[_indices_len];
	memcpy(indices,_indices,sizeof(unsigned int)*_indices_len);
	singleSided = _singleSided;
}

void RT_Mesh::ClearMesh()
{
	free(indices);
	free(vertices);
}
/*
bool RT_Mesh::shadowRayHitTriangle(std::vector<glm::vec3> _triangle, Ray *ray, bool _singleSided, float& distance, glm::vec3 & PHit, float min_dist)
{//TODO edit to use references or pointers?
	//std::cout << "kreslim vertexy: v0: " << glm::to_string(triangle[0]) << " a v1: " << glm::to_string(triangle[1]) << " a v2: " << glm::to_string(triangle[2]) << " \n";
	glm::vec3 normal = RT_Mesh::getTriangleNormal(_triangle);//is normalized
	//std::cout <<"normala: "<< glm::to_string(normal) << " vertex 0 = " << glm::to_string(_triangle[0])<<"\n";
	float d = getDistanceFromOrigin(normal, _triangle[0]);
	int e = 0;
	 d = getPlaneIntersectionDistance(d, normal, ray->origin, ray->direction, _singleSided,e);
	///Distance > min_dist <-this triangle is further than previously hit
	if (d <= 0.001f ||d > min_dist||e==1) {
		return false; // Triangle is behind the camera OR it's parallel with the ray OR it's faced the other way and is single sided - both invisible ///TODO too far - fake dust in air
	}
	glm::vec3 _PHit = getPlaneIntersection(ray->origin, d, ray->direction);

	///is Phit inside the triangle? test against each edge
	if (glm::dot(normal, glm::cross(_triangle[1] - _triangle[0], _PHit - _triangle[0])) < 0) { return false; }
	if (glm::dot(normal, glm::cross(_triangle[2] - _triangle[1], _PHit - _triangle[1])) < 0) { return false; }
	if (glm::dot(normal, glm::cross(_triangle[0] - _triangle[2], _PHit - _triangle[2])) < 0) { return false; }
	PHit = _PHit;
	distance = d;
	return true; //successful hit
}*/

//min_distance is lastly hit triangle PHit distance
bool RT_Mesh::rayHitTriangle(std::vector<glm::vec3> _triangle,bool isPrimary, Ray *ray, bool _singleSided,float& distance, glm::vec3 & PHit,float min_dist)
{//TODO edit to use references or pointers?
	glm::vec3 normal = RT_Mesh::getTriangleNormal(_triangle);//is normalized
	float d = getDistanceFromOrigin(normal, _triangle[0]);
	int e = 0;
	d/*parametr t z P=O+tR*/ = (isPrimary ? 1.f : -1.f)* getPlaneIntersectionDistance(isPrimary,d, normal, ray->origin, ray->direction, _singleSided,e);
	///Distance > min_dist <-this triangle is further than previously hit
	//d *= (isPrimary? 1.f : -1.f);
	//std::cout << "vzdalenost: " << d << std::endl;
	if (d <= (isPrimary? CAM_NEAR_PLANE: 0.f) +0.001f || d > min_dist||e==1/*d==0*/) {//e==1 odvraceny trojuhelnik,
		return false; // Triangle is behind the camera OR it's parallel with the ray OR it's faced the other way and is single sided - both invisible
	}
	glm::vec3 _PHit = getPlaneIntersection(ray->origin, d, ray->direction);

	///is Phit inside the triangle? test against each edge
	if (glm::dot(normal, glm::cross(_triangle[1] - _triangle[0], _PHit - _triangle[0])) < 0) { return false; }
	if (glm::dot(normal, glm::cross(_triangle[2] - _triangle[1], _PHit - _triangle[1])) < 0) { return false; }
	if (glm::dot(normal, glm::cross(_triangle[0] - _triangle[2], _PHit - _triangle[2])) < 0) { return false; }
	PHit = _PHit;
	distance = d;
	return true; //successful hit
}

RT_Mesh::~RT_Mesh()
{
	ClearMesh();
}

bool RT_Mesh::VerticesAreUnique(float* vertices,unsigned int vertex_count)
{
	/*glm::vec3 vertex;
	for(int i = 0; i<vertex_count; i+=3)
	{
		glm::vec3(vertices[i], vertices[i + 1], vertices[i + 2]);
	}

	return true;*/
	//bit array?,, hash? sort? find all X-values, for all the same ones find those with same Y, then Z
	return true;
}
