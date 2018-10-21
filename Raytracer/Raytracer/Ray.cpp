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

glm::vec3 Ray::calcRayPerspectiveDirection( float x, float y,float w, float near,Camera camera)
{
	glm::vec3 ray_direction = glm::normalize(glm::vec4(camera.camera_position,w) - glm::transpose(camera.view_matrix)*glm::vec4( glm::vec3(x,y,-1.0*near),w));
	return ray_direction;
}
