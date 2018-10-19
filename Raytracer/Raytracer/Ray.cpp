#include "Ray.h"
#include <iostream>
#define GLM_ENABLE_EXPERIMENTAL
#include "GLM\gtx\string_cast.hpp"

Ray::Ray()
{
}


Ray::~Ray()
{
}

glm::vec3 Ray::calcRayPerspectiveDirection( float x, float y,float w, int width, int height, float near,Camera camera)
{
	//std::cout << "camera position: "<< glm::to_string(camera.camera_position)<<"pixel: "<<glm::to_string(glm::vec3(x,y,near)) << std::endl;
	//top left origin
	//x = (-1.f + ((COORDS_FLOAT_WIDTH / (float)width)*(float)x));// *camera.scale);
	//y = (-1.f + ((COORDS_FLOAT_HEIGHT / (float)height)*(float)y));// *camera.scale) * 1 / camera.aspect_ratio;

	glm::vec3 ray_direction = glm::normalize(glm::vec3(x,y,-1.0*near) - (glm::vec3(0)- camera.camera_position));

	//return glm::vec3(glm::vec3((-width/2)+x,(-height/2)+y,near) - origin);
	//return glm::vec3(origin - glm::vec3(x, y, near));
	//glm::vec4 pixel = /*c*amera.projection_matrix**/camera.view_matrix*glm::vec4(x, y, near, w);
	//std::cout << "x: "<< std::to_string(x) << "y: "<< std::to_string(y) << std::endl;
	//std::cout << " ray direction: " << glm::to_string(ray_direction) << std::endl;
	return ray_direction;
	//return glm::normalize(glm::vec3(glm::vec3(x+camera.view_matrix[2][0], y + camera.view_matrix[2][1], near)-camera.camera_position));
	//return glm::vec3(glm::normalize(pixel - glm::vec4(camera.camera_position,w) ) );
	
	//return glm::normalize(glm::vec4(glm::vec4(x, y , near,w) - glm::vec4(camera.camera_position,w)) );
	//return glm::normalize( /*camera.projection_matrix**/(camera.projection_matrix*camera.view_matrix *glm::vec4(glm::vec4(x, y, near, w))) - glm::vec4(camera.camera_position,w));
}
