#pragma once
#define GLM_LEFT_HANDED
#include <GLM\glm.hpp>
#include "Camera.h"
#include "RT_Mesh.h"
class Ray
{
	/**
	*From https://github.com/lkesteloot/prism/blob/master/Vec3.cpp#L128
 ** which is from: https://www.johndcook.com/wavelength_to_RGB.html
 */
	glm::vec3 wavelength2rgb(int wavelength) {
		float red, green, blue;

		if (wavelength >= 380 && wavelength < 440) {
			red = -(wavelength - 440) / (440 - 380.);
			green = 0.0;
			blue = 1.0;
		}
		else if (wavelength >= 440 && wavelength < 490) {
			red = 0.0;
			green = (wavelength - 440) / (490 - 440.);
			blue = 1.0;
		}
		else if (wavelength >= 490 && wavelength < 510) {
			red = 0.0;
			green = 1.0;
			blue = -(wavelength - 510) / (510 - 490.);
		}
		else if (wavelength >= 510 && wavelength < 580) {
			red = (wavelength - 510) / (580 - 510.);
			green = 1.0;
			blue = 0.0;
		}
		else if (wavelength >= 580 && wavelength < 645) {
			red = 1.0;
			green = -(wavelength - 645) / (645 - 580.);
			blue = 0.0;
		}
		else if (wavelength >= 645 && wavelength < 781) {
			red = 1.0;
			green = 0.0;
			blue = 0.0;
		}
		else {
			red = 0.0;
			green = 0.0;
			blue = 0.0;
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

		return glm::vec3(red*factor, green*factor, blue*factor);
	}
public:
	Ray();
	~Ray();

	glm::vec3 origin;
	glm::vec3 direction;
	glm::vec3 hit_normal;
	glm::vec3 inv_dir;
	glm::vec3 wavelength;
	bool sign[3];

	float	prev_D; //if primary, set, if shadow - read

	static glm::vec3 calcRayDirection(glm::vec3 origin, glm::vec3 target);
	static void calcRayPerspectiveDirection(glm::vec3 &origin, glm::vec3 &dir, float x, float y, float near, Camera &camera) {
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

	/*bool intersectBB(glm::vec3(&bounds)[2], float &t) {
		glm::vec3 t0 = (bounds[0] - origin)*inv_dir;
		glm::vec3 t1 = (bounds[1] - origin)*inv_dir;
		glm::vec3 tmin = glm::min(t0, t1);
	

		return true;
	}*/

	bool intersectBB( glm::vec3(&bounds)[2],float &t) {
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

	bool intersectTriangleMT(bool isPrimary, RT_Mesh::Vertex* _triangle, bool _singleSided, glm::vec3 &PHit, glm::vec3 & NHit, float &t, float &u, float &v, float min_dist);
	bool intersectSphericalLight(glm::vec3 &dir, glm::vec3 &orig, glm::vec3 &center, float &radius2, glm::vec3 &PHit, glm::vec3 &NHit, float &min_dist,float &t);
};

