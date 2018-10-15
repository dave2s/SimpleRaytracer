#include "Ray.h"



Ray::Ray()
{
}


Ray::~Ray()
{
}

glm::vec3 Ray::calcRayPerspectiveDirection(glm::vec3 origin, int x, int y, int width, int height, int near)
{
	return glm::vec3(glm::vec3((-width/2)+x,(-height/2)+y,near) - origin);
}
