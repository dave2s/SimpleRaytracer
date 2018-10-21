#include "Camera.h"
#include <string>
#include <iostream>
#include "GLM\gtc\matrix_transform.hpp"



Camera::Camera(glm::vec3 origin, float _fovy, float _aspect_ratio)
{
	aspect_ratio = _aspect_ratio;
	camera_position = glm::vec3(origin);
	view_matrix = glm::lookAt(origin,origin+CAM_NEAR_PLANE,glm::vec3(0.f,1.f,0.f));
	//std::cout << "Aspect ratio:" << std::to_string(TO_RADIANS(fovy)) << std::endl;
	projection_matrix = glm::perspective(TO_RADIANS(fovy),_aspect_ratio,CAM_NEAR_PLANE, CAM_FAR_PLANE);
	fovy = _fovy;
	scale = glm::tan(TO_RADIANS(fovy*0.5));
	CalcCamView(glm::vec3(0.f,0.f,-1.0f));
}

Camera::~Camera()
{
}
///TODO Spojit Proj * View
void Camera::CalcCamView(glm::vec3 camera_front)
{
	//glm::vec3 cameraDirection = glm::normalize(camera_position - camera_front);
	view_matrix = glm::lookAt(camera_position, camera_position + camera_front, glm::vec3(0.f, 1.f, 0.f));
}
