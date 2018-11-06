#include "RT_Mesh.h"

RT_Mesh::RT_Mesh()
{
	indices_len = 0;
	singleSided = false;
	glm::mat4 object_to_world;
}

void RT_Mesh::CreateMesh(const float *_vertices, const unsigned int *_indices, unsigned int _vertices_len, unsigned int _indices_len, bool _singleSided, glm::f32vec3 _color,float _albedo) {
	color = _color;
	indices_len = _indices_len;
	vertices = new float[_vertices_len];
	memcpy(vertices,_vertices,sizeof(float)*_vertices_len);	
	indices = new unsigned int[_indices_len];
	memcpy(indices,_indices,sizeof(unsigned int)*_indices_len);
	singleSided = _singleSided;
	albedo = glm::f32vec3(_albedo);
	material = _material;
}

void RT_Mesh::ClearMesh()
{
	delete(indices);
	delete(vertices);
	//delete(this);
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

bool RT_Mesh::intersectTriangle(bool isPrimary, glm::vec3* _triangle, bool _singleSided, Ray *ray, glm::vec3 &PHit, float &t, float &u, float &v,float &min_dist) {
	glm::fvec3 N = getTriangleUnNormal(_triangle);
	//float triangle_area = N.length()/2.f; //plocha Rovnobezniku/2 dana vektorovym soucinem dvou vektoru (nenormovana normala)
	//std::cout << triangle_area << " << tohle by nemelo hazet kraviny" << std::endl;
	//v = N.(AB x AP) / N.N
	float NN = glm::dot(N, N);//N.N

	///Calc PHit 
	//Let's ommit H's functions and code everything again in here..hopefully helps
	//check for parallelism
	float RdotN = glm::dot(N, ray->direction);
	//if (isPrimary) RdotN*=(-1.0f);

	if ((0.0001f > glm::abs(RdotN)) || (RdotN > 0 && _singleSided)) {//parallel or facing other way
		return false;
	}

	float d = glm::dot(N, _triangle[0]); // using first vertex by convention, distance to triangle plane

	t = (glm::dot(N, ray->origin) + d)/RdotN;
	if (t < 0 + (isPrimary ? CAM_NEAR_PLANE : 0.001f) || t>min_dist) return false; //triangle is behind the origin
	
	PHit = ray->origin + t * ray->direction; //P=O+tR parametricke vyjadreni primky, t je vzdalenost od O po smeru R

	///je v trojuhelniku?

	glm::vec3 edge = _triangle[1] - _triangle[0];
	glm::vec3 vp = PHit - _triangle[0];
	glm::vec3 C = glm::cross(edge, vp); //kolmobezka s rovinou trojuhelniku
	if (glm::dot(N, C) < 0) return false; //

	edge = _triangle[2] - _triangle[1];
	vp = PHit - _triangle[1];
	C = glm::cross(edge, vp);
	u = glm::dot(N, C);
	if (u < 0) return false;

	edge = _triangle[0] - _triangle[2];
	vp = PHit - _triangle[2];
	C = glm::cross(edge, vp);
	v = glm::dot(N, C);
	if (v < 0) return false;
	
	u /= NN; v /= NN;
	return true;
}
//Moller-Trumbore
///Split this into primary and secondary function - optimize
//float margin = 0.001f;
bool RT_Mesh::intersectTriangleMT(bool isPrimary, glm::vec3* _triangle, bool _singleSided, Ray *ray, glm::vec3 &PHit,glm::vec3 & NHit, float &t, float &u, float &v, float &min_dist) {
	glm::vec3 edge01 = _triangle[1] - _triangle[0];
	glm::vec3 edge02 = _triangle[2] - _triangle[0];
	glm::vec3 pvec = glm::cross(ray->direction, edge02);
	float D = glm::dot(edge01, pvec);
	
	if (isPrimary) {
		if ((D < 0.001f && _singleSided)) {
			return false;
		} // 0 backfacing, close to zero miss
		//if (glm::abs(D) < 0.0001f) return false;//ortho, parallel with normal
	}
	else {
		if ( (D > -0.001f) && (D < 0.001f) ) return false;
	}
	glm::vec3 tvec = ray->origin - _triangle[0];
	float D_inv = 1 / D;
	u = glm::dot(tvec, pvec) * D_inv;
	if (u < 0 || u>1) { return false; }

	tvec = glm::cross(tvec, edge01);
	v = glm::dot(ray->direction, tvec)*D_inv;
	if (v < 0 || u + v>1) { return false; }

	t = glm::dot(edge02, tvec) * D_inv;
	if (isPrimary && ((t < CAM_NEAR_PLANE) || (t > min_dist))) {
			PHit = ray->origin + t * ray->direction;
			return false;
	}
	else if (-1.f*t > min_dist) {
		return false;
	}
	else if (t < 0.001f)
	{		
		if (((t < 0.001f)&&(t>-0.01f)) && (((ray->prev_D < 0.f) && (D < 0.f)) || ((ray->prev_D > 0.f) && (D > 0.001f))))// goto jmp;
		{
		}
		else {
			PHit = ray->origin + t * ray->direction;
			return false;
		}
	}
	PHit = ray->origin + t * ray->direction;
	NHit = glm::normalize(glm::cross(edge01, edge02));
	return true;
}

//min_distance is lastly hit triangle PHit distance
bool RT_Mesh::rayHitTriangle(glm::vec3* _triangle,bool isPrimary, Ray *ray, bool _singleSided,float& distance, glm::vec3 & PHit,float min_dist)
{//TODO edit to use references or pointers?
	glm::vec3 normal = RT_Mesh::getTriangleNormal(_triangle);//is normalized
	float d = getDistanceFromOrigin(normal, _triangle[0]);
	int e = 0;
	d/*parametr t z P=O+tR*/ = (isPrimary ? 1.f : -1.f)* getPlaneIntersectionDistance(isPrimary,d, normal, ray->origin, ray->direction, _singleSided,e);
	///Distance > min_dist <-this triangle is further than previously hit
	d *= (isPrimary? 1.f : -1.f);//camera has negative z, ray doesnt...
	//std::cout << "vzdalenost: " << d << std::endl;
	if ((d <= (isPrimary? CAM_NEAR_PLANE: 0.f) +0.00f) || (d > min_dist)||(e==1)/*d==0*/) {//e==1 odvraceny trojuhelnik,
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
