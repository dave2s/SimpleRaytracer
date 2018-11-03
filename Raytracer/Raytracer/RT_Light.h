#pragma once
#include <glm/glm.hpp>
#ifndef M_PI
#define M_PI 3.14159265358979323846264f
#endif

class RT_Light
{
public:

	enum LIGHT_TYPE {none = 0, point = 1, distant=2};

	RT_Light(float intensity, glm::f32vec3 color);
	virtual ~RT_Light();

	glm::vec3 color;
	float intensity;

	virtual void shine(glm::vec3& light_intensity, float& light_distance, glm::vec3& light_dir, glm::vec3& P);
	virtual LIGHT_TYPE getType() { return none; }
};

class RT_PointLight : public RT_Light
{
	
public:
	glm::vec3 position;
	RT_PointLight(glm::vec3 pos, float intensity, glm::f32vec3 color);

	void shine(glm::vec3& light_intensity, float& light_distance, glm::vec3& light_dir, glm::vec3& P);
	LIGHT_TYPE getType() { return point; }
};

class RT_DistantLight : public RT_Light
{
	
public:
	glm::vec3 direction;
	RT_DistantLight(glm::vec3 dir, float intensity, glm::f32vec3 color);

	void shine(glm::vec3& light_intensity, float& light_distance, glm::vec3& light_dir, glm::vec3& P);
	LIGHT_TYPE getType() { return distant; }
};
