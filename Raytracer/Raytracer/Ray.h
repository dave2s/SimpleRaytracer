#pragma once
#define GLM_LEFT_HANDED
#include <GLM\glm.hpp>
#include "Camera.h"
#include <algorithm>
#include <vector>
//#include "Defines.h"
#define WIDEN_CONSTANT 5.f
//static const std::vector<int> wavelengths_intervals{ 380,440,490,510,580,645,781 };
static const std::vector<int> wavelengths_intervals{ 400,440,490,510,580,645,700 };

class Ray
{
public:
	Ray();
	~Ray();

	glm::f32vec3 origin;
	glm::f32vec3 direction;
	glm::f32vec3 hit_normal;
	glm::f32vec3 inv_dir;
	const float tmax = inf;
	bool isMonochrom;
	int wavelength;
	bool sign[3];

	float	prev_D; //if primary, set, if shadow - read

	struct Hitinfo {
		glm::f32vec3 PHit;
		glm::f32vec3 NHit;
		uint32_t tri_idx;
		float u; float v;
	};

	static glm::vec3 calcRayDirection(glm::vec3 origin, glm::vec3 target);
	static void calcRayPerspectiveDirection(glm::vec3 &origin, glm::vec3 &dir, float x, float y, float near, Camera &camera)
	{
		origin = glm::inverse(camera.view_matrix)*glm::vec4(0.f, 0.f, 0.f, 1.f);
		dir = glm::normalize(glm::vec3(glm::inverse(camera.view_matrix)*glm::vec4(x, y, -near, 1.f)) - origin);
		return;
	}
	static void calcReflectedDirection(glm::vec3 &NHit, glm::vec3 & raydir) {
		raydir = raydir - 2.f * NHit*(glm::dot(NHit, raydir));
	}

	void precomputeValues() {
		inv_dir = 1.f / direction;

		sign[0] = inv_dir.x < 0.f;
		sign[1] = inv_dir.y < 0.f;
		sign[2] = inv_dir.z < 0.f;
	}

	static float norm(glm::vec3 vec) { return (vec[0] * vec[0] + vec[1] * vec[1] + vec[2] * vec[2]); }

	//True -refracted, false- reflection
	static glm::vec3 refract(const float& ior, glm::vec3& direction, glm::vec3& hit_normal) {
		float ior_in = 1;
		float ior_out = ior;
		glm::vec3 n = hit_normal;
		float cos_in = glm::clamp(-1.f, 1.f, glm::dot(direction, n));

		if (cos_in < 0) {
			cos_in = -cos_in;
		}
		else {
			std::swap(ior_in, ior_out);
			n = -n;
		}
		//ratio of light propagation in incoming space / outcoming space
		float  in_out_ratio = ior_in / ior_out;
		float k = 1 - in_out_ratio * in_out_ratio * (1 - cos_in * cos_in);
		glm::vec3 out = (k < 0.f) ?
			glm::vec3(0) : in_out_ratio * direction + (in_out_ratio * cos_in - glm::sqrt(k))*n;
		return out;
	}

	static void fresnel(const float& ior, float& kr, glm::vec3& direction, glm::vec3& hit_normal) {
		float ior_in = 1;
		float ior_out = ior;
		//glm::vec3 n = hit_normal;
		float cos_in = glm::clamp(-1.f, 1.f, glm::dot(direction, hit_normal));

		if (cos_in > 0) {
			std::swap(ior_in, ior_out);
		}
		//sin_out = ior1/ior2  *   sqrt(1-cos_in^2) = (ior1/ior2)*sin_in
		float sin_out = ior_in / ior_out * std::sqrtf(std::max(0.f, 1 - cos_in * cos_in));

		///(ior1/ior2)*sin_in = sin_out
		//if sin_out>1, total internal reflection - light is not transmitted
		if (sin_out >= 1) {
			kr = 1;
		}
		else {//cos^2 = 1 - sin^2
			float cos_out = std::sqrtf(std::max(0.f, 1 - sin_out * sin_out));
			cos_in = std::fabsf(cos_in);

			float Rs = ((ior_out * cos_in) - (ior_in * cos_out))
				/ ((ior_out*cos_in) + (ior_in*cos_out));

			float Rp = ((ior_in * cos_in) - (ior_out * cos_out))
				/ ((ior_in * cos_in) + (ior_out * cos_out));

			kr = (Rs * Rs + Rp * Rp) / 2;
		}
	}

	/*from
	*https://github.com/lkesteloot/prism
	*/
	static float iorFromWavelength(float wavelength, float B, float C){
	// https://en.wikipedia.org/wiki/Cauchy%27s_equation

	// Widen rainbow, renormalize B.
	float new_C = C * WIDEN_CONSTANT;// WIDEN_CONSTANT;
	B = B + C / (.540f*.540f) - new_C / (.540f*.540f);
	C = new_C;

	float wl_um = wavelength / 1000.0f;
	return B + (C / (wl_um*wl_um));
/*
		wavelength *= 0.001f;
		return B + ((C*0.001f) / (wavelength*wavelength));*/
	}


	static glm::vec3 wavelength2rgb2(int l) {
			double t;  double r = 0.0; double g = 0.0; double b = 0.0;
			if ((l >= 400.0) && (l < 410.0)) { t = (l - 400.0) / (410.0 - 400.0); r = +(0.33*t) - (0.20*t*t); }
			else if ((l >= 410.0) && (l < 475.0)) { t = (l - 410.0) / (475.0 - 410.0); r = 0.14 - (0.13*t*t); }
			else if ((l >= 545.0) && (l < 595.0)) { t = (l - 545.0) / (595.0 - 545.0); r = +(1.98*t) - (t*t); }
			else if ((l >= 595.0) && (l < 650.0)) { t = (l - 595.0) / (650.0 - 595.0); r = 0.98 + (0.06*t) - (0.40*t*t); }
			else if ((l >= 650.0) && (l < 700.0)) { t = (l - 650.0) / (700.0 - 650.0); r = 0.65 - (0.84*t) + (0.20*t*t); }
			if ((l >= 415.0) && (l < 475.0)) { t = (l - 415.0) / (475.0 - 415.0); g = +(0.80*t*t); }
			else if ((l >= 475.0) && (l < 590.0)) { t = (l - 475.0) / (590.0 - 475.0); g = 0.8 + (0.76*t) - (0.80*t*t); }
			else if ((l >= 585.0) && (l < 639.0)) { t = (l - 585.0) / (639.0 - 585.0); g = 0.84 - (0.84*t); }
			if ((l >= 400.0) && (l < 475.0)) { t = (l - 400.0) / (475.0 - 400.0); b = +(2.20*t) - (1.50*t*t); }
			else if ((l >= 475.0) && (l < 560.0)) { t = (l - 475.0) / (560.0 - 475.0); b = 0.7 - (t)+(0.30*t*t); }
			return glm::f32vec3(r,g,b);
	}

	/**From https://github.com/lkesteloot/prism
	 * From: https://www.johndcook.com/wavelength_to_RGB.html
	 */
	static glm::vec3 wavelength2rgb(int wavelength) {
		float red, green, blue;

		if (wavelength >= 380.f && wavelength < 440.f) {
			red = -(wavelength - 440.f) / (440.f - 380.f);
			green = 0.0f;
			blue = 1.0f;
		}
		else if (wavelength >= 440.f && wavelength < 490.f) {
			red = 0.0f;
			green = (wavelength - 440.f) / (490.f - 440.f);
			blue = 1.0f;
		}
		else if (wavelength >= 490.f && wavelength < 510.f) {
			red = 0.0f;
			green = 1.0f;
			blue = -(wavelength - 510.f) / (510.f - 490.f);
		}
		else if (wavelength >= 510.f && wavelength < 580.f) {
			red = (wavelength - 510.f) / (580.f - 510.f);
			green = 1.0f;
			blue = 0.0f;
		}
		else if (wavelength >= 580.f && wavelength < 645.f) {
			red = 1.0f;
			green = -(wavelength - 645.f) / (645.f - 580.f);
			blue = 0.0f;
		}
		else if (wavelength >= 645.f && wavelength < 781.f) {
			red = 1.0f;
			green = 0.0f;
			blue = 0.0f;
		}
		else {
			red = 0.0f;
			green = 0.0f;
			blue = 0.0f;
		}

		// Let the intensity fall off near the vision limits.
		float factor;
		if (wavelength >= 380.f && wavelength < 420.f) {
			factor = 0.3f + 0.7f*(wavelength - 380.f) / (420.f - 380.f);
		}
		else if (wavelength >= 420.f && wavelength < 701.f) {
			factor = 1.0f;
		}
		else if (wavelength >= 701.f && wavelength < 781.f) {
			factor = 0.3f + 0.7f*(780.f - wavelength) / (780.f - 700.f);
		}
		else {
			factor = 0.0f;
		}

		return glm::f32vec3(red*factor, green*factor, blue*factor);
	}

	bool intersectBB(const glm::vec3(&bounds)[2],float &t) {
		float tx0; float ty0; float tx1; float ty1;
		
		tx0 = (bounds[sign[0]].x - origin.x) * inv_dir.x;
		tx1 = (bounds[1-sign[0]].x - origin.x) * inv_dir.x;
		ty0 = (bounds[sign[1]].y - origin.y) * inv_dir.y;
		ty1 = (bounds[1 - sign[1]].y - origin.y) * inv_dir.y;

		if ((tx0 > ty1) || (ty0 > tx1)) return false;
		if (ty0 > tx0) tx0 = ty0;
		if (ty1 < tx1) tx1 = ty1;

		float tz0; float tz1;
		tz0 = (bounds[sign[2]].z-origin.z)*inv_dir.z;
		tz1 = (bounds[1-sign[2]].z - origin.z)*inv_dir.z;
		
		if ((tx0 > tz1) || (tz0 > tx1)) return false;
		if (tz0 > tx0) tx0 = tz0;
		if (tz1 < tx1) tx1 = tz1;

		t = tx0;
		if (t < 0) {
			t = tx1;
			if (t < 0) return false;
		}

		return true;
	}

};