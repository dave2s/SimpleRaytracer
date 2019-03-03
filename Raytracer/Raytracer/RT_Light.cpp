#include "RT_Light.h"
#include "Ray.h"
#include <algorithm>


/**From: https://github.com/lkesteloot/prism
 * From: https://www.johndcook.com/wavelength_to_RGB.html
 *//*
inline glm::vec3 RT_Light::wavelen2rgb(int wavelength)
{
	float r, g, b;

	if (wavelength >= 380 && wavelength < 440) {
		r = -(wavelength - 440) / (440 - 380.);
		g = 0.0;
		b = 1.0;
	}
	else if (wavelength >= 440 && wavelength < 490) {
		r = 0.0;
		g = (wavelength - 440) / (490 - 440.);
		b = 1.0;
	}
	else if (wavelength >= 490 && wavelength < 510) {
		r = 0.0;
		g = 1.0;
		b = -(wavelength - 510) / (510 - 490.);
	}
	else if (wavelength >= 510 && wavelength < 580) {
		r = (wavelength - 510) / (580 - 510.);
		g = 1.0;
		b = 0.0;
	}
	else if (wavelength >= 580 && wavelength < 645) {
		r = 1.0;
		g = -(wavelength - 645) / (645 - 580.);
		b = 0.0;
	}
	else if (wavelength >= 645 && wavelength < 781) {
		r = 1.0;
		g = 0.0;
		b = 0.0;
	}
	else {
		r = 0.0;
		g = 0.0;
		b = 0.0;
	}

	// Let the intensity fall off near the vision limits.
	float factor;
	if (wavelength >= 380 && wavelength < 420) {
		factor = 0.3 + 0.7*(wavelength - 380) / (420 - 380.);
	}
	else if (wavelength >= 420 && wavelength < 701) {
		factor = 1.0;
	}
	else if (wavelength >= 701 && wavelength < 781) {
		factor = 0.3 + 0.7*(780 - wavelength) / (780 - 700.);
	}
	else {
		factor = 0.0;
	}

	return glm::vec3(r*factor, g*factor, b*factor);
}*/

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
	_position = _initialPosition  =pos;
}
void RT_PointLight::resetPosition()
{
	_position = _initialPosition;
}

void RT_PointLight::shine(glm::f32vec3& light_intensity, float& light_distance, glm::vec3& light_dir, glm::vec3& PHit) {
	float distance_squared;
	light_dir = (PHit-_position);
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
	_direction = _initialDirection = dir;
}
void RT_DistantLight::resetDirection()
{
	_direction = _initialDirection;
}

void RT_DistantLight::shine(glm::vec3& light_intensity,float& light_distance, glm::vec3& light_dir, glm::vec3& P) {
	light_dir = _direction;
	light_intensity = color * intensity;
	light_distance = inf;
	//NHit, shadow_ray->direction)
	//hit_color = (albedo)*intensity*glm::vec3(light->color)*std::max(0.f, NdotRay);
	//glm::f32vec3(pixel_color)*((*light)->color)*(hit_mesh->albedo / glm::f32vec3(M_PI)) * glm::f32vec3(intensity)*std::max(0.f, glm::dot(NHit, shadow_ray->direction));
}