#include "RT_Light.h"
#include "Ray.h"
#include <algorithm>




RT_Light::RT_Light(float _intensity, glm::f32vec3 col)
{
	intensity = _intensity;
	color = col;
}

void RT_Light::shine(glm::vec3& light_intensity, float& light_distance, glm::vec3& light_dir, glm::vec3& P)
{
}


RT_Light::~RT_Light()
{
}



RT_PointLight::RT_PointLight(glm::vec3 pos, float _intensity, glm::f32vec3 col) :RT_Light(_intensity, col)
{
	position = pos;
}
void RT_PointLight::shine(glm::vec3& light_intensity, float& light_distance, glm::vec3& light_dir, glm::vec3& PHit) {
	float distance_squared;
	light_dir = (PHit-position);
	distance_squared = Ray::norm(light_dir);
	light_intensity = color*intensity / glm::vec3((float)(4.f*M_PI*(distance_squared)));
	light_distance = sqrt(distance_squared);
	//vyuziju potreby vzdalenosti a znormalizuju smer svetla - asi nejrychlejsi reseni opet vitezi jiz objevene kolo
	light_dir[0] /= light_distance; light_dir[1] /= light_distance; light_dir[2] /= light_distance;
	//NHit, shadow_ray->direction)
	//hit_color = (albedo)*intensity*glm::vec3(light->color)*std::max(0.f, NdotRay);
	//glm::f32vec3(pixel_color)*((*light)->color)*(hit_mesh->albedo / glm::f32vec3(M_PI)) * glm::f32vec3(intensity)*std::max(0.f, glm::dot(NHit, shadow_ray->direction));
}

RT_DistantLight::RT_DistantLight(glm::vec3 dir, float _intensity, glm::f32vec3 col) : RT_Light(_intensity, col)
{
	direction = dir;
}
void RT_DistantLight::shine(glm::vec3& light_intensity,float& light_distance, glm::vec3& light_dir, glm::vec3& P) {
	light_dir = direction;
	light_intensity = color * intensity;
	light_distance = inf;
	//NHit, shadow_ray->direction)
	//hit_color = (albedo)*intensity*glm::vec3(light->color)*std::max(0.f, NdotRay);
	//glm::f32vec3(pixel_color)*((*light)->color)*(hit_mesh->albedo / glm::f32vec3(M_PI)) * glm::f32vec3(intensity)*std::max(0.f, glm::dot(NHit, shadow_ray->direction));
}