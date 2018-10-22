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
	static void calcRayPerspectiveDirection(Ray *ray,float x, float y, float w, float near, Camera camera);
};

