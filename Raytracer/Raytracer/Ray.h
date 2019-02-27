#pragma once
#define GLM_LEFT_HANDED
#include <GLM\glm.hpp>
#include "Camera.h"
#include "RT_Mesh.h"
#include <algorithm>
class Ray
{
public:
	Ray();
	~Ray();

	glm::f32vec3 origin;
	glm::f32vec3 direction;
	glm::f32vec3 hit_normal;
	glm::f32vec3 inv_dir;
	bool sign[3];

	float	prev_D; //if primary, set, if shadow - read

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

	/*bool intersectBB(glm::vec3(&bounds)[2], float &t) {
		glm::vec3 t0 = (bounds[0] - origin)*inv_dir;
		glm::vec3 t1 = (bounds[1] - origin)*inv_dir;
		glm::vec3 tmin = glm::min(t0, t1);
	

		return true;
	}*/
	//True -refracted, false- reflection
	glm::vec3 refract(/*glm::vec3& dirOut,*/const float& ior){
		float ior_in = 1;
		float ior_out = ior;
		glm::vec3 n = this->hit_normal;
		float cos_in = glm::clamp(-1.f, 1.f, glm::dot(this->direction, n));

		if(cos_in < 0) {
			cos_in = -cos_in;
		}
		else {
			std::swap(ior_in, ior_out);
			n = -n;
		}
		//ratio of light propagation in incoming space / outcoming space
		float  in_out_ratio = ior_in / ior_out;
		float k = 1 - in_out_ratio * in_out_ratio * (1 - cos_in * cos_in);
		glm::vec3 out  = (k < 0.f) ?
			glm::vec3(0) : in_out_ratio * this->direction + (in_out_ratio * cos_in - glm::sqrt(k))*n;
		return out;
		//return (k<0.f) ?  
			//glm::vec3(0) : in_out_ratio * this->direction + (in_out_ratio * cos_in - glm::sqrt(k))*n;
	}

	static void fresnel(const float& ior, float& kr, glm::vec3& direction, glm::vec3& hit_normal){
		float ior_in = 1;
		float ior_out = ior;
		glm::vec3 n = hit_normal;
		float cos_in = glm::clamp(-1.f, 1.f, glm::dot(direction, n));

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

