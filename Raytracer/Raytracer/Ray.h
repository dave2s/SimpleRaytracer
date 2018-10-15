#pragma once
#include <GLM\glm.hpp>
class Ray
{
public:
	Ray();
	~Ray();

	glm::vec3 calcRayPerspectiveDirection(glm::vec3 origin, float x, float y, int width, int height, float near);
};

