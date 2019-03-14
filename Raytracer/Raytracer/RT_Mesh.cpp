#include "RT_Mesh.h"
#include "Defines.h"

void GetHitProperties(Vertex& v0, Vertex& v1, Vertex& v2, float& u, float& v, int& textureHeight, int& textureWidth, glm::vec3& N, glm::vec2 &texture_coords)
{
#ifndef SMOOTH_SHADING
	N = getTriangleNormal(v0.position, v1.position, v2.position);
#else
	//Interpolate vertex normals using barycentric coordinates.
	//Perspective correction: divide by respective v.z then multiply by fragment.z
	N = glm::normalize((1 - u - v)*v0.normal + u*v1.normal + v*v2.normal);
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

void GetHitProperties(Vertex& v0, Vertex& v1, Vertex& v2,float u, float v, glm::vec3& N)
{
#ifndef SMOOTH_SHADING
	N = getTriangleNormal(v0.position, v1.position, v2.position);
#else
	//Interpolate vertex normals using barycentric coordinates.
	//Perspective correction: divide by respective v.z then multiply by fragment.z
	///Presumption assimp generated normals....should check whether normals are present, if not, has no point
	N = glm::normalize((1 - u - v)*v0.normal + u * v1.normal + v * v2.normal);
#endif
}

//Moller-Trumbore
///Split this into primary and secondary function - optimize
//float margin = 0.001f;
static bool intersectTriangleMT(Ray* ray, bool isPrimary, Vertex& v0, Vertex& v1, Vertex& v2, bool _singleSided, glm::vec3 &PHit, glm::vec3 & NHit, float &t, float &u, float &v, float max_dist) {
	glm::vec3 edge01 = (v1).position - (v0).position;
	glm::vec3 edge02 = v2.position - v0.position;
	glm::vec3 pvec = glm::cross(ray->direction, edge02);
	float D = glm::dot(edge01, pvec);

	if (isPrimary) {
		if ((D < 0.001f && _singleSided) || (glm::abs(D) < 0.0001f)) {
			return false;
		} // 0 backfacing, close to zero miss
		//if (glm::abs(D) < 0.0001f) return false;//ortho, parallel with normal
	}
	else {
		if ((D > -0.0001f) && (D < 0.0001f)) return false;
	}
	glm::vec3 tvec = ray->origin - v0.position;
	float D_inv = 1 / D;
	u = glm::dot(tvec, pvec) * D_inv;
	if (u < 0 || u>1) { return false; }

	tvec = glm::cross(tvec, edge01);
	v = glm::dot(ray->direction, tvec)*D_inv;
	if (v < 0 || u + v>1) { return false; }

	t = glm::dot(edge02, tvec) * D_inv;
	if ((t < 0) || (t > max_dist)) {
		//PHit = ray->origin + t * ray->direction;
		return false;
	}
	/*else if (t > min_dist) {
		return false;
	}*/
	else if (t < 0.001f)
	{
		if (((t < 0.001f) && (t > -0.01f)) && (((ray->prev_D < 0.f) && (D < 0.f)) || ((ray->prev_D > 0.f) && (D > 0.001f))))// goto jmp;
		{
		}
		else {
			//PHit = ray->origin + t * ray->direction;
			return false;
		}
	}
	PHit = ray->origin + t * ray->direction;
	NHit = glm::normalize(glm::cross(edge01, edge02));
	return true;
}


RT_Mesh::RT_Mesh()
{

}

RT_Mesh::RT_Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices, bool singleSided, Material material, float albedo, MATERIAL_TYPE material_type, std::vector<Texture> textures)
	: _albedo(albedo),
	_material(material),
	_material_type(material_type),
	_textures(textures),
	_indices(indices),
	//_indices_len(indices_len),
	_vertices(vertices),
	_triangle_count(indices.size() / 3),
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



void RT_Mesh::updateBoundaries(Vertex &vertex) {
	for (unsigned char i = 0; i < (char)vertex.position.length(); ++i) {
		if (vertex.position[i] < boundary_points[0][i]) {
			boundary_points[0][i] = vertex.position[i];
		}
		if (vertex.position[i] > boundary_points[1][i]) {
			boundary_points[1][i] = vertex.position[i];
		}
	}
}

void RT_Mesh::ClearMesh()
{
	//delete(indices);
	//delete(vertices);
	//delete(this);
}

void RT_Mesh::computeBounds(const glm::f32vec3 &planeNormal, float &dnear, float &dfar) const
{
	float d;
	for (uint32_t i = 0; i < _vertices_len; ++i) {
		d = glm::dot(planeNormal, _vertices[i].position);
		if (d < dnear) dnear = d;
		if (d > dfar) dfar = d;
	}
}

bool RT_Mesh::intersect_triangle(Ray* ray, uint32_t triangle, float& t_near, Ray::Hitinfo& info)const {
	float u_prim, v_prim;
	glm::f32vec3 PHit; float PHit_dist = inf;
	glm::f32vec3 NHit; float max_dist = t_near;
	bool intersected = false;
	
	Vertex v0 = _vertices[_indices[0 + 3 * triangle]];
	Vertex v1 = _vertices[_indices[1 + 3 * triangle]];
	Vertex v2 = _vertices[_indices[2 + 3 * triangle]];
	if (intersectTriangleMT(ray, true,
		v0, v1, v2,
		_singleSided,
		PHit,
		NHit,
		PHit_dist,
		u_prim,
		v_prim,
		max_dist) && PHit_dist < t_near)
	{
		info.NHit = ray->hit_normal = NHit; //NHit changes with calculations, N_hit is transfered to the next section as last normal
		info.u = u_prim; info.v = v_prim;
		info.tri_idx = triangle;
		t_near = PHit_dist;
		info.PHit = PHit;
		intersected = true;
	}

	return intersected;
}

//t_near comes with maximum distance to trace
//returns nearest hit
bool RT_Mesh::intersect(Ray* ray, float& t_near, Ray::Hitinfo& info) const
{
	//if(mesh->material == RT_Mesh::DIFFUSE)
		//glm::vec3 NHit;
	float u_prim, v_prim;
	glm::f32vec3 PHit; float PHit_dist=inf;
	glm::f32vec3 NHit; float max_dist = t_near;
	bool intersected = false;
	//uint32_t triangle_count = this.getTriangleCount();
	for (uint32_t idx = 0; idx < _triangle_count; ++idx) {///For every triangle of the mesh
		Vertex v0 = _vertices[_indices[0 + 3 * idx]];
		Vertex v1 = _vertices[_indices[1 + 3 * idx]];
		Vertex v2 = _vertices[_indices[2 + 3 * idx]];
		if (intersectTriangleMT(ray, true,
			v0,v1,v2,
			_singleSided,
			PHit,
			NHit,
			PHit_dist,
			u_prim,
			v_prim,
			max_dist) && PHit_dist<t_near) 
		{
			info.NHit = ray->hit_normal = NHit; //NHit changes with calculations, N_hit is transfered to the next section as last normal
			info.u = u_prim; info.v = v_prim;
			info.tri_idx = idx;
			t_near = PHit_dist;
			info.PHit = PHit;
			intersected = true;
		}
	}
	return intersected;

}

	RT_Mesh::~RT_Mesh()
{
	ClearMesh();
}

