#include "Ray.h"
#include <iostream>
#define GLM_ENABLE_EXPERIMENTAL
#include "GLM\gtx\string_cast.hpp"

Ray::Ray()
{
}


Ray::~Ray()
{
}

glm::vec3 Ray::calcRayDirection(glm::vec3 origin, glm::vec3 target) {
	return glm::normalize(target - origin);
}

void Ray::calcRayPerspectiveDirection(Ray *ray, float x, float y,float w, float near,Camera camera)
{
	//glm::vec3 ray_direction = glm::normalize(/*glm::vec4(camera.camera_position,w) -*/ camera.view_matrix*glm::vec4( glm::vec3(x,y,near),w));
	glm::vec4 ray_origin = glm::inverse(camera.view_matrix)*glm::vec4(0.f, 0.f, 0.f,0.0f);
	ray->direction = glm::normalize(ray_origin-(glm::inverse(camera.view_matrix)*glm::vec4(glm::vec3(x, y, near), w)));
	ray->origin = ray_origin;
	return;// ray_direction;
}
