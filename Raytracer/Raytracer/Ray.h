#pragma once
#define GLM_LEFT_HANDED
#include <GLM\glm.hpp>
#include "Camera.h"
#include "RT_Mesh.h"
class Ray
{
public:
	Ray();
	~Ray();

	glm::vec3 origin;
	glm::vec3 direction;
	glm::vec3 hit_normal;
	glm::vec3 inv_dir;
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
	//True -refracted, false- reflection
	bool refract(glm::vec3& dirOut,const float& ior){
		float angle_in = 1;
		float angle_out = ior;
		glm::vec3 n = this->hit_normal;
		float angle_in_cosine = glm::clamp(-1.f, 1.f, glm::dot(this->direction, n));

		if(angle_in_cosine < 0) {
			angle_in_cosine = -angle_in_cosine;
		}
		else {
			std::swap(angle_in, angle_out);
			n = -n;
		}

		float  ni_over_nt = angle_in / angle_out;
		float k = 1 - ni_over_nt * ni_over_nt * (1 - angle_in_cosine * angle_in_cosine);

		if (k>0.f) {
			dirOut = ni_over_nt * this->direction + (ni_over_nt * angle_in_cosine - glm::sqrt(k))*n;
			return true;
		}
		dirOut = glm::vec3(0);
		return false;
	}

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

	bool intersectTriangleMT(bool isPrimary, Vertex* _triangle, bool _singleSided, glm::vec3 &PHit, glm::vec3 & NHit, float &t, float &u, float &v, float min_dist);
};

