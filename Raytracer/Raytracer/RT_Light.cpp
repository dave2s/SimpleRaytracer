#include "RT_Light.h"
#include "Ray.h"
#include <algorithm>



RT_Light::RT_Light(glm::vec3 pos, float _intensity, glm::vec3 col)
{
	position = pos;
	intensity = _intensity;
	color = col;
}

void RT_Light::shine(glm::vec3 &intensity,RT_Light *light,glm::vec3 &P,glm::f32vec3 & hit_color,glm::f32vec3 albedo,float NdotRay) {
	intensity = glm::vec3(light->intensity) /** glm::vec3(light->color)*/ / glm::vec3((float)(4.f*M_PI*(Ray::norm(light->position - P))));

	hit_color = (albedo)*intensity*glm::vec3(light->color)*std::max(0.f,NdotRay);
	//glm::f32vec3(pixel_color)*((*light)->color)*(hit_mesh->albedo / glm::f32vec3(M_PI)) * glm::f32vec3(intensity)*std::max(0.f, glm::dot(NHit, shadow_ray->direction));
}

RT_Light::~RT_Light()
{
}

RT_DistantLight::RT_DistantLight(glm::vec3 pos, float _intensity, glm::vec3 col)
{
	position = pos;
	intensity = _intensity;
	color = col;
}

RT_PointLight::RT_PointLight(glm::vec3 pos, float _intensity, glm::vec3 col)
{
	position = pos;
	intensity = _intensity;
	color = col;
}
