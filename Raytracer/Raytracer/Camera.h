#pragma once
#include "GLM\glm.hpp"

#define CAM_NEAR_PLANE 0.1f
#define CAM_FAR_PLANE 100.0f

class Camera
{
public:
	Camera(glm::vec3 origin);
	~Camera();

	glm::mat4 view_matrix;
	glm::vec3 camera_position;

	void CalcCamView(glm::vec3 camera_target);
	/*float near_plane = NEAR_PLANE;
	float far_plane = FAR_PLANE;*/

private:

};

