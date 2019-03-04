#include "Ray.h"
#define GLM_ENABLE_EXPERIMENTAL

#ifdef DEBUG
#define GLM_ENABLE_EXPERIMENTAL
#include "GLM\gtx\string_cast.hpp"
#endif // DEBUG



Ray::Ray()
{
	this->isMonochrom = false;
}


Ray::~Ray()
{

}

glm::vec3 Ray::calcRayDirection(glm::vec3 origin, glm::vec3 target) {
	return glm::normalize(target - origin);
}

