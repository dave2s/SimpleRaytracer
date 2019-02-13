#include "Ray.h"
#include <iostream>
#define GLM_ENABLE_EXPERIMENTAL

#ifdef DEBUG
#define GLM_ENABLE_EXPERIMENTAL
#include "GLM\gtx\string_cast.hpp"
#endif // DEBUG



Ray::Ray()
{
}


Ray::~Ray()
{

}

glm::vec3 Ray::calcRayDirection(glm::vec3 origin, glm::vec3 target) {
	return glm::normalize(target - origin);
}

//Moller-Trumbore
///Split this into primary and secondary function - optimize
//float margin = 0.001f;
bool Ray::intersectTriangleMT(bool isPrimary, RT_Mesh::Vertex* _triangle, bool _singleSided, glm::vec3 &PHit, glm::vec3 & NHit, float &t, float &u, float &v, float min_dist) {
	glm::vec3 edge01 = (_triangle[1]).position - (_triangle[0]).position;
	glm::vec3 edge02 = _triangle[2].position - _triangle[0].position;
	glm::vec3 pvec = glm::cross(direction, edge02);
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
	glm::vec3 tvec = origin - _triangle[0].position;
	float D_inv = 1 / D;
	u = glm::dot(tvec, pvec) * D_inv;
	if (u < 0 || u>1) { return false; }

	tvec = glm::cross(tvec, edge01);
	v = glm::dot(direction, tvec)*D_inv;
	if (v < 0 || u + v>1) { return false; }

	t = glm::dot(edge02, tvec) * D_inv;
	if ((t < 0) || (t > min_dist)) {
		//PHit = ray->origin + t * ray->direction;
		return false;
	}
	/*else if (t > min_dist) {
		return false;
	}*/
	else if (t < 0.001f)
	{
		if (((t < 0.001f) && (t > -0.01f)) && (((prev_D < 0.f) && (D < 0.f)) || ((prev_D > 0.f) && (D > 0.001f))))// goto jmp;
		{
		}
		else {
			//PHit = ray->origin + t * ray->direction;
			return false;
		}
	}
	PHit = origin + t * direction;
	NHit = glm::normalize(glm::cross(edge01, edge02));
	return true;
}

bool Ray::intersectSphericalLight(glm::vec3 &dir, glm::vec3 &orig, glm::vec3 &center, float &radius2, glm::vec3 &PHit, glm::vec3 &NHit, float &min_dist, float &t) {
	float t0, t1;
#if 1
		// geometric solution
	glm::vec3 L = center - orig;
	float tca = glm::dot(L,dir);
	// if (tca < 0) return false;
	float d2 = glm::dot(L,L) - tca * tca;
	if (d2 > radius2) return false;
	float thc = sqrt(radius2 - d2);
	t0 = tca - thc;
	t1 = tca + thc;
#else 
		// analytic solution
	glm::vec3 L = orig - center;
	float a = glm::dot(dir,dir);
	float b = 2 * glm::dot(dir,L);
	float c = glm::dot(L,L) - radius2;
	if (!solveQuadratic(a, b, c, t0, t1)) return false;
#endif 
	if (t0 > t1) std::swap(t0, t1);

	if (t0 < 0) {
		t0 = t1; // if t0 is negative, let's use t1 instead 
		if (t0 < 0) return false; // both t0 and t1 are negative 
	}

	t = t0;

	return true;
}

/*
void Ray::calcRayPerspectiveDirection(glm::vec3 &origin,glm::vec3 &dir, float x, float y, float near,Camera &camera)
{
	origin = glm::inverse(camera.view_matrix)*glm::vec4(0.f, 0.f, 0.f, 1.f);
	dir = glm::normalize(glm::vec3(glm::inverse(camera.view_matrix)*glm::vec4(x, y, -near, 1.f)) - origin);
	return;
}
*/
