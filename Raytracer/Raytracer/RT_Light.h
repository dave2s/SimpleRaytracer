#pragma once
#include <glm/glm.hpp>
#ifndef M_PI
#define M_PI 3.14159265358979323846264f
#endif

class RT_Light
{
public:

	enum LIGHT_TYPE { point, distant};

	RT_Light(glm::vec3 pos, float intensity, glm::vec3 color);
	~RT_Light();

	glm::vec3 position;
	glm::vec3 color;
	float intensity;

	static void shine(glm::vec3 &intensity, RT_Light *light, glm::vec3 &P, glm::f32vec3 & hit_color, glm::f32vec3 albedo, float NdotRay);
};

class RT_PointLight : public RT_Light
{
public:
	RT_PointLight(glm::vec3 pos, float intensity, glm::vec3 color);

};

class RT_DistantLight : public RT_Light
{
public:
	RT_DistantLight(glm::vec3 pos, float intensity, glm::vec3 color);

};
