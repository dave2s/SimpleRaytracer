#pragma once
#include <glm/glm.hpp>
class RT_Light
{
public:
	RT_Light(glm::vec3 pos, float intensity, glm::vec3 color);
	~RT_Light();

	glm::vec3 position;
	glm::vec3 color;
	float intensity;
};

