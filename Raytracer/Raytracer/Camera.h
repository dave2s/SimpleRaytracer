#pragma once
#define GLM_LEFT_HANDED
#include "GLM\glm.hpp"

#define CAM_NEAR_PLANE 0.5f
#define CAM_FAR_PLANE 1000.0f
#define COORDS_FLOAT_WIDTH 2.0f
#define COORDS_FLOAT_HEIGHT 2.0f
#define PI 3.14159265358979323846264f
#define TO_RADIANS(x)  x*PI/180.f

const float inf = std::numeric_limits<float>::max();

class Camera
{
public:
	Camera(glm::vec3 origin, glm::vec3 front, float fovy, float aspect_ratio);
	~Camera();

	//glm::mat4 view_matrix;
	glm::vec3 camera_position;
	glm::mat4 projection_matrix;
	float aspect_ratio;
	float fovy;
	float scale;

	void Update(glm::vec3 direction);
	/*float near_plane = NEAR_PLANE;
	float far_plane = FAR_PLANE;*/

private:

};

