#include "Camera.h"
#include "GLM\gtc\matrix_transform.hpp"


Camera::Camera(glm::vec3 origin)
{
	camera_position = origin;
	view_matrix = glm::lookAt(origin,origin+CAM_NEAR_PLANE,glm::vec3(0.f,1.f,0.f));
}

Camera::~Camera()
{
}

void Camera::CalcCamView(glm::vec3 camera_target)
{
	glm::vec3 cameraDirection = glm::normalize(camera_position - camera_target);
	view_matrix = glm::lookAt(camera_position,camera_position+cameraDirection*CAM_NEAR_PLANE,glm::vec3(0.f,1.f,0.f));
}
