#include "RT_Mesh.h"




RT_Mesh::RT_Mesh()
{
	index_count = 0;
	singleSided = false;
}

void RT_Mesh::CreateMesh(float *_vertices, unsigned int *_indices, unsigned int _vertex_count, unsigned int _index_count, bool _singleSided) {
	index_count = _index_count;
	vertices = new float[_vertex_count];
	memcpy(vertices,_vertices,sizeof(float)*_vertex_count);	
	vertex_count = _vertex_count;
	indices = new unsigned int[_index_count];
	memcpy(indices,_indices,sizeof(unsigned int)*_index_count);
	singleSided = _singleSided;
}

void RT_Mesh::ClearMesh()
{

}

bool RT_Mesh::rayHitTriangle(std::vector<glm::vec3> _triangle, Ray *ray, Camera camera, bool _singleSided)
{//TODO edit to use references or pointers?
	//std::cout << "kreslim vertexy: v0: " << glm::to_string(triangle[0]) << " a v1: " << glm::to_string(triangle[1]) << " a v2: " << glm::to_string(triangle[2]) << " \n";
	glm::vec3 normal = RT_Mesh::getTriangleNormal(_triangle);//is normalized
	//std::cout <<"normala: "<< glm::to_string(normal) << " vertex 0 = " << glm::to_string(_triangle[0])<<"\n";
	float d = getDistanceFromOrigin(normal, _triangle[0]);
	float hit_distance = getPlaneIntersectionDistance(d, normal, ray->origin, ray->direction, _singleSided);
	
	if (hit_distance <= CAM_NEAR_PLANE/*&&hit_distance==0*/) {
		return false; // Triangle is behind the camera OR it's parallel with the ray OR it's faced the other way and is single sided - both invisible ///TODO too far - fake dust in air
	}
	glm::vec3 Phit = getPlaneIntersection(ray->origin, hit_distance, ray->direction);

	///is Phit inside the triangle? test against each edge
	if (glm::dot(normal, glm::cross(_triangle[1] - _triangle[0], Phit - _triangle[0])) < 0) { return false; }
	if (glm::dot(normal, glm::cross(_triangle[2] - _triangle[1], Phit - _triangle[1])) < 0) { return false; }
	if (glm::dot(normal, glm::cross(_triangle[0] - _triangle[2], Phit - _triangle[2])) < 0) { return false; }

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
