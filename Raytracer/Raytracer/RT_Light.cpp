#include "RT_Light.h"
#include "Ray.h"



RT_Light::RT_Light(glm::vec3 pos, float _intensity, glm::vec3 col)
{
	position = pos;
	intensity = _intensity;
	color = col;

}

void RT_Light::shine(glm::vec3 &intensity,RT_Light *light,glm::vec3 &P) {
	intensity = glm::vec3(light->intensity) * glm::vec3(light->color) / glm::vec3((float)(4.f*M_PI*(Ray::norm(light->position - P))));
}

RT_Light::~RT_Light()
{
}
