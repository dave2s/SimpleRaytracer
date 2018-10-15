#pragma once
#include <GLM\glm.hpp>
class Ray
{
public:
	Ray();
	~Ray();

	glm::vec3 calcRayPerspectiveDirection(glm::vec3 origin, int x, int y, int width, int height, int near);
};

