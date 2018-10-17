#include "Ray.h"

Ray::Ray()
{
}


Ray::~Ray()
{
}

glm::vec3 Ray::calcRayPerspectiveDirection(glm::vec3 origin, float x, float y, int width, int height, float near,Camera camera)
{
	//return glm::vec3(glm::vec3((-width/2)+x,(-height/2)+y,near) - origin);
	//return glm::vec3(origin - glm::vec3(x, y, near));
	return glm::normalize(glm::vec3(glm::vec3(x+camera.view_matrix[2][0], y + camera.view_matrix[2][1], near)-camera.camera_position));
}
