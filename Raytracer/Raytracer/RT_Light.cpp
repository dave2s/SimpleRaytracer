#include "RT_Light.h"



RT_Light::RT_Light(glm::vec3 pos, float _intensity, glm::vec3 col)
{
	position = pos;
	intensity = _intensity;
	color = col;
}


RT_Light::~RT_Light()
{
}
