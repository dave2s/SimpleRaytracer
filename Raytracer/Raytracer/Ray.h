#pragma once
#include <GLM\glm.hpp>
#include "Camera.h"
class Ray
{
public:
	Ray();
	~Ray();

	glm::vec3 calcRayPerspectiveDirection(float x, float y,float w, int width, int height, float near, Camera camera);
};

