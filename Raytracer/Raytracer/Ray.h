#pragma once
#define GLM_LEFT_HANDED
#include <GLM\glm.hpp>
#include "Camera.h"
class Ray
{
public:
	Ray();
	~Ray();

	glm::vec3 origin;
	glm::vec3 direction;
	glm::vec3 hit_normal;
	float	prev_D; //if primary, set, if shadow - read

	static glm::vec3 calcRayDirection(glm::vec3 origin, glm::vec3 target);
	static void calcRayPerspectiveDirection(glm::vec3 &origin, glm::vec3 &dir,float x, float y, float near, Camera &camera);
	static void calcReflectedDirection(glm::vec3 &NHit, glm::vec3 & raydir) { 
		raydir = raydir - 2.f * NHit*(glm::dot(NHit, raydir));
		int a=1;
	}

	static float norm(glm::vec3 vec) { return (vec[0] * vec[0] + vec[1] * vec[1] + vec[2] * vec[2]); }
};

