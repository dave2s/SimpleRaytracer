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
bool Ray::intersectTriangleMT(bool isPrimary, Vertex* _triangle, bool _singleSided, glm::vec3 &PHit, glm::vec3 & NHit, float &t, float &u, float &v, float min_dist) {
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

/*
void Ray::calcRayPerspectiveDirection(glm::vec3 &origin,glm::vec3 &dir, float x, float y, float near,Camera &camera)
{
	origin = glm::inverse(camera.view_matrix)*glm::vec4(0.f, 0.f, 0.f, 1.f);
	dir = glm::normalize(glm::vec3(glm::inverse(camera.view_matrix)*glm::vec4(x, y, -near, 1.f)) - origin);
	return;
}
*/
