#include <iostream>
#include <string>
#include <vector>
#include "SDL.h"
#include <SDL_pixels.h>
#include <SDL_render.h>
//#define GLM_FORCE_CUDA
#define GLM_LEFT_HANDED
#define GLM_FORCE_RADIANS
//#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <stdlib.h>
#include "Camera.h"
#include "RT_Mesh.h"
#include "RT_Light.h"
#include "Ray.h"
#include "Bunny.h"

//Enable external gpu on a laptop supporting nvidia optimus
#include <Windows.h>
extern "C" {
	_declspec(dllexport) DWORD NvOptimusEnablement = 0x00000001;
}
#define WIDTH 320.f
#define HEIGHT 240.f

#define AMBIENT_LIGHT_STRENGHT 0.3f

const float toRadians = glm::pi<float>() / 180.0f;

std::vector<RT_Mesh*> mesh_list;
std::vector<RT_Light*> light_list;

SDL_Surface* CreateRGBImage(int width, int height) {
	return SDL_CreateRGBSurface(0, width, height, 32, (Uint32)0xff000000, (Uint32)0x00ff0000, (Uint32)0x0000ff00, (Uint32)0x000000ff);
}

void setRGBAPixel(SDL_Surface* rendered_image,int x, int y, glm::u8vec4 rgba) {
	//rendered_image->pixels[x + 640 * y] = 
	unsigned char* pixels = (unsigned char*)rendered_image->pixels;
	pixels[4 * (y*rendered_image->w + x) + 0] = rgba[0];//blue
	pixels[4 * (y*rendered_image->w + x) + 1] = rgba[1];//green
	pixels[4 * (y*rendered_image->w + x) + 2] = rgba[2];//red
	pixels[4 * (y*rendered_image->w + x) + 3] = rgba[3];//alpha
}

void CreatePointLight(glm::vec3 pos, float intensity, glm::vec3 color) {
	RT_Light* point_light = new RT_Light(pos,intensity, color);
	light_list.push_back(point_light);
}

void CreateBox() {

	const float vertices[] = {
		//predni stena
		-0.5f, 0.5f, 1.f,
		0.5f, 0.5f, 1.f,
		0.5f, -0.5f, 1.f,
		-0.5f, -0.5f, 1.f,
		//zadni stena
		-0.5f, 0.5f, 2.f,
		0.5f, 0.5f, 2.f,
		0.5f, -0.5f, 2.f,
		-0.5f, -0.5f, 2.f
	};

	const unsigned int indices[] = {
		//predni stena
		//0,1,2,
		//0,2,3,
		//zadni stena
		5,4,7,
		5,7,6,
		//bocni prava stena
		1,5,6,
		1,6,2,
		//bocni leva stena
		4,0,3,
		4,3,7,
		//horni trojuhelnik
		3,2,6
	};

	glm::u8vec4 color = glm::u8vec4(1,255,1,255);

	RT_Mesh* box = new RT_Mesh();		//indices vertices length pointer arithmetic 
	box->CreateMesh(vertices, indices, *(&vertices+1)-vertices, *(&indices + 1) - indices, false,color);
	mesh_list.push_back(box);

	RT_Mesh* bunny = new RT_Mesh();
	Bunny actual_bunny;
	bunny->CreateMesh(actual_bunny.vertices, actual_bunny.indices, actual_bunny.vertex_count, actual_bunny.index_count, true, glm::u8vec4(255,1,1,255));

}

int main(int argc, char* argv[])
{
	SDL_Window* main_window;
	SDL_Event event;
	bool quit = false;
	float width_step = COORDS_FLOAT_WIDTH / (float)WIDTH;
	float height_step = COORDS_FLOAT_HEIGHT / (float)HEIGHT;

	///CAMERA
	Camera camera = Camera(glm::vec3(0.0f,0.0f, 0.0f),45.f, (float)WIDTH/ (float)HEIGHT);

	SDL_Init(SDL_INIT_VIDEO);
	main_window = SDL_CreateWindow("Raytracer",SDL_WINDOWPOS_UNDEFINED,SDL_WINDOWPOS_UNDEFINED,WIDTH,HEIGHT,SDL_WINDOW_OPENGL);
	if (main_window==NULL) {
		std::cerr << "SDL2 Main window creation failed.";
		return 1;
	}

	SDL_Renderer* renderer = SDL_CreateRenderer(main_window, -1, 0);
	SDL_Surface* frame_buffer = CreateRGBImage(WIDTH,HEIGHT);
	if (frame_buffer == NULL) {
		std::cout << "Failed create rgb surface";
		return 2;
	}

	CreateBox();
	CreatePointLight(glm::vec3(0.0f, -1.5f, 1.5f), 3.33f,glm::vec3(255,255,255));

	//for each pixel of the frame buffer set it's color
	//SDL_Rect dstrect = { 5, 5, 320, 240 };
	int triangle_count;
	int i;
	float view_x;
	float view_y;
	//SDL_Delay(30000);
	SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, frame_buffer);
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
	SDL_RenderClear(renderer);
	SDL_RenderCopy(renderer, texture, NULL, NULL);
	SDL_RenderPresent(renderer);

	Ray *primary_ray;
	float min_dist = inf;
	float PHit_dist;
	glm::vec3 PHit = glm::vec3(-1);
	glm::vec3 closest_PHit = PHit;

	while (!quit)
	{
		SDL_WaitEvent(&event);

		switch (event.type)
		{
		case SDL_QUIT:
			quit = true;
			break;
		}
		
		for (int y = 0; y < HEIGHT; ++y) {
			for (int x = 0; x < WIDTH; ++x) {
				view_x = (-1.f + (width_step*(float)x)) *camera.scale;
				view_y = (1.f - (height_step*(float)y)) *camera.scale * 1 / camera.aspect_ratio;

				min_dist = inf;
				///For every mesh stored
				__int64 mesh_iter_index = -1;
				int mesh_triangle_index = -1;

				for (auto mesh = mesh_list.begin(); mesh != mesh_list.end(); ++mesh) {
					triangle_count = (*mesh)->getTriangleCount();//std::cout << std::to_string(triangle_count) << "\n";
					for (i = 0; i < triangle_count; ++i) {///For every triangle of the mesh
						primary_ray = new Ray();
						(Ray::calcRayPerspectiveDirection(primary_ray,
							view_x,
							view_y,
							1.0,
							CAM_NEAR_PLANE, camera));
						if (RT_Mesh::rayHitTriangle((*mesh)->getTriangle(i),
							primary_ray, (*mesh)->isSingleSided(), PHit_dist,PHit, min_dist)) {
							if (PHit_dist < min_dist) {
								mesh_iter_index = mesh - mesh_list.begin();
								mesh_triangle_index = i;
								min_dist = PHit_dist;
								closest_PHit = PHit;
								//std::cout << "closest_PHit:" << glm::to_string(closest_PHit) << std::endl;
								//std::cout << "PHit distance:" << std::to_string(min_dist) << std::endl;
							}
						}
					}
				}
				if (!(mesh_iter_index == -1 || mesh_triangle_index == -1)) {
					PHit = closest_PHit;
					//setRGBAPixel(frame_buffer, x, y, 0, 255, 0, 255);
					Ray *shadow_ray = new Ray();
					bool is_lit = true;
					RT_Mesh *hit_mesh = *(mesh_list.begin() + mesh_iter_index);
					hit_mesh->getTriangle(mesh_triangle_index);
					//for each light cast shadow ray

					///TODO GET OBJECT COLOR
					//glm::u8vec4 pixel_color = glm::vec3(mesh->color[0], mesh->color[1], mesh->color[2]) * glm::vec3(AMBIENT_LIGHT_STRENGHT);
					glm::u8vec4 pixel_color = glm::f32vec4(hit_mesh->color) * glm::f32vec4(AMBIENT_LIGHT_STRENGHT, AMBIENT_LIGHT_STRENGHT, AMBIENT_LIGHT_STRENGHT, 1.0f);

					for (auto light = light_list.begin(); light != light_list.end(); ++light) {
						//std::cout << "prochazim svetla z trefeneho pruseciku: "<<glm::to_string(closest_PHit)<<std::endl;
						shadow_ray->direction = glm::normalize((*light)->position - closest_PHit);
						shadow_ray->origin = closest_PHit;
						//we won't intersect object further than this distance of light and the last hit
						min_dist = RT_Mesh::getDistanceFromOrigin(shadow_ray->direction, (*light)->position);
						//for each object
						for (auto mesh = mesh_list.begin(); mesh != mesh_list.end(); ++mesh) {
							//std::cout << "prochazim meshe" << std::endl;
							if (!is_lit) { std::cout << "opoustim smycku meshu - nasel jsem stin" << std::endl; break; }
							triangle_count = (*mesh)->getTriangleCount();//std::cout << std::to_string(triangle_count) << "\n";
							for (i = 0; i < triangle_count; ++i) {///For every triangle of the mesh
								//std::cout << "prochazim trojuhelniky" << std::endl;
								if (RT_Mesh::rayHitTriangle((*mesh)->getTriangle(i),
									shadow_ray,	false,PHit_dist, PHit, min_dist)) {
									is_lit = false;
									//std::cout << "Nasel jsem trojuhelnik po ceste od PHitu k svetlu, skacu ze smycky trojuhelniku" << std::endl;
									break;
								}
								else {
									//is_lit = true;
								}

							}// end for each triangle of mesh
						}//end for each mesh in the scene
						if (is_lit) {
							//std::cout << "OSVETLUJI PIXEL" << std::endl;
							///TODO FOR EACH LIGHT HIT (path must be uninterrupted)
							/// !!! IMPLICIT CAST TO UINT FROM FLOAT !!!
							pixel_color = glm::f32vec4(pixel_color) * glm::f32vec4((*light)->intensity, (*light)->intensity, (*light)->intensity, 1.0f); //glm::u8vec4(glm::f32vec4((0.f * (*light)->intensity), (127.f * (*light)->intensity), (0.f * (*light)->intensity), 0));
							//std::cout << "OSVETLUJI PIXEL :" << glm::to_string(pixel_color) << std::endl;
						}
					}//end for each light in the scene
					setRGBAPixel(frame_buffer, x, y, pixel_color);
				}
			}//end for each pixel
			SDL_UpdateTexture(texture, NULL, frame_buffer->pixels, WIDTH * sizeof(Uint32));//
			SDL_RenderCopy(renderer, texture, NULL, NULL);
			SDL_RenderPresent(renderer);
		}
		std::cout << "\n KONEC KRESLENI\n";
		SDL_Delay(3000);
	}

	SDL_DestroyTexture(texture);
	SDL_FreeSurface(frame_buffer);
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(main_window);
	SDL_Quit();

	return 0;
}

