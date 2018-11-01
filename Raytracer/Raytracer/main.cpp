#include <iostream>
#include <iomanip>
#include <string>
#include <vector>
#include <chrono>
#include "SDL.h"
#include <SDL_pixels.h>
#include <SDL_render.h>
#include <algorithm>
//#define GLM_FORCE_CUDA
#define GLM_LEFT_HANDED
#define GLM_FORCE_RADIANS
//#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
//#include <glm/gtx/norm.hpp>
#include <stdlib.h>
#include "Camera.h"
#include "RT_Mesh.h"
#include "RT_Light.h"
#include "Ray.h"
#include "Bunny.h"

#define SCREEN_SPACE_SUBSAMPLE 2
#define PROFILING
//#define FINAL_RENDER

//Enable external gpu on a laptop supporting nvidia optimus
/*#include <Windows.h>
extern "C" {
	_declspec(dllexport) DWORD NvOptimusEnablement = 0x00000001;
}*/
#ifndef FINAL_RENDER

#define WIDTH 400.f
#define HEIGHT 260.f
#else
#define WIDTH 800.f
#define HEIGHT 600.f
#endif

// mark functions returning via reference
#define OUT

#define MAX_DEPTH 2
#define AMBIENT_LIGHT_STRENGHT 0.f

bool quit = false;

//const float toRadians = glm::pi<float>() / 180.0f;


const float LO0x = -2;
const float LO0y = -2;
const float LO0z = -2;
const float HI0x = 2;
const float HI0y = 2;
const float HI0z = 2;

std::vector<RT_Mesh*> mesh_list;
std::vector<RT_Light*> light_list;

SDL_Surface* CreateRGBImage(int width, int height) {
	return SDL_CreateRGBSurface(0, width, height, 32, (Uint32)0xff000000, (Uint32)0x00ff0000, (Uint32)0x0000ff00, (Uint32)0x000000ff);
}

void setRGBAPixel(SDL_Surface* rendered_image,int x, int y, glm::u8vec3 rgba) {
	//rendered_image->pixels[x + 640 * y] = 
	unsigned char* pixels = (unsigned char*)rendered_image->pixels;
	pixels[4 * (y*rendered_image->w + x) + 0] = rgba[2];//blue
	pixels[4 * (y*rendered_image->w + x) + 1] = rgba[1];//green
	pixels[4 * (y*rendered_image->w + x) + 2] = rgba[0];//red
	pixels[4 * (y*rendered_image->w + x) + 3] = glm::u8(255);//rgba[3];//alpha
}

void CreatePointLight(glm::vec3 pos, float intensity, glm::vec3 color) {
	RT_PointLight* point_light = new RT_PointLight(pos,intensity, color);
	light_list.push_back(point_light);
}

void CreateGlobalLight(glm::vec3 direction, float intensity, glm::vec3 color) {
	RT_DistantLight* global_light = new RT_DistantLight(direction, intensity, color);
	light_list.push_back(global_light);
}

glm::u8vec3 quantize(glm::f32vec3 fcolor) {
	glm::f32vec3 qcolor;

	return glm::u8vec3(qcolor);
}

void CreateWall() {
	float vertices[] = {
		//zadni stena
		-10.f, -10.f, -10.f,
		10.f, -10.f, -10.f,
		10.f, 10.f, -10.f,
		-10.f, 10.f, -10.f
	};



	unsigned int indices[] = {
		//zadni stena
		0,1,2,
		0,2,3,
	};

	//glm::u8vec4 color = glm::u8vec4(1, 1, 255, 255);
	glm::u8vec4 color = glm::u8vec4(255, 255, 255, 255);

	RT_Mesh* plane = new RT_Mesh();		//indices vertices length pointer arithmetic 
	plane->CreateMesh(vertices, indices, *(&vertices + 1) - vertices, *(&indices + 1) - indices, false, color,0.2f);
	mesh_list.push_back(plane);
}

void CreateBox() {

	float vertices[] = {
		//predni stena
		-1.f, -1.f, -2.5f,
		1.f, -1.f, -2.5f,
		1.f, 1.f, -2.5f,
		-1.f, 1.f, -2.5f,
		//zadni stena
		-1.f, -1.f, -5.f,
		1.f, -1.f, -5.f,
		1.f, 1.f, -5.f,
		-1.f, 1.f, -5.f
	};



	unsigned int indices[] = {
		//predni stena
		0,1,2,
		0,2,3,
		//zadni stena
		5,4,7, //7,4,5,
		5,7,6,
		//bocni prava stena
		1,5,6,
		1,6,2,
		//bocni leva stena
		4,0,3,
		4,3,7,
		//horni trojuhelnik
		3,2,6,
		3,6,7,
		//spodni trojuhelnik
		1,0,4,
		1,4,5

	};

	glm::u8vec4 color = glm::u8vec4(255,255,255,255);

	RT_Mesh* box = new RT_Mesh();		//indices vertices length pointer arithmetic 
	box->CreateMesh(vertices, indices, *(&vertices+1)-vertices, *(&indices + 1) - indices, false, color,0.2f);
	mesh_list.push_back(box);

	/*RT_Mesh* bunny = new RT_Mesh();
	bunny->CreateMesh(bunny_vertices, bunny_indices, bunny_vertex_count, bunny_index_count, true, glm::u8vec4(255,1,1,255));
	mesh_list.push_back(bunny);*/
}

int app_exit(int return_code,SDL_Texture* texture, SDL_Renderer* renderer, SDL_Surface* frame_buffer, SDL_Window* main_window)
{
	std::cout << "Ragequit();" << std::endl;
	SDL_DestroyTexture(texture);
	SDL_FreeSurface(frame_buffer);
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(main_window);
	SDL_Quit();
	//exit(0);
	return return_code;
}

glm::vec3 calcRandPos(float LO0x, float LO0y,float LO0z, float HI0x, float HI0y, float HI0z) {
	float x = LO0x + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / (HI0x - LO0x)));
		//SDL_Delay(2);
		float y = LO0y + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / (HI0y - LO0x)));
		//SDL_Delay(3);
		float z = LO0z + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / (HI0z - LO0x)));
		std::cout << "x: " << x << " y: " << y << "z: " << z << std::endl;
		return glm::vec3(x,y,z);
}

void MovePolling(SDL_Event &event,Camera &camera) {
	//SDL_PollEvent(&event);
	//bool left_click;

	if (event.type == SDL_KEYDOWN) {
		switch (event.key.keysym.sym) {
		case SDLK_w:
			camera.camera_position[2] += 0.1f;
			break;
		case SDLK_s:
			camera.camera_position[2] -= 0.1f;
			break;
		case SDLK_a:
			camera.camera_position[0] -= 0.1f;
			break;
		case SDLK_d:
			camera.camera_position[0] += 0.1f;
			break;
		case SDLK_LEFT:
			if (!light_list.empty())
				((RT_PointLight*)light_list.at(0))->position[0] -= 1.0f;
			break;
		case SDLK_RIGHT:
			if (!light_list.empty())
				((RT_PointLight*)light_list.at(0))->position[0] += 1.0f;
			break;
		case SDLK_UP:
			if (!light_list.empty())
				((RT_PointLight*)light_list.at(0))->position[1] += 1.0f;
			break;
		case SDLK_DOWN:
			if (!light_list.empty())
				((RT_PointLight*)light_list.at(0))->position[1] -= 1.0f;
			break;
		case SDLK_c:
			camera.camera_position[1] -= 0.1f;
			break;
		case SDLK_r:
			camera.camera_position = { 0.f ,0.f,0.0f};
			break;
		case SDLK_SPACE:
			camera.camera_position[1] += 0.1f;
			break;
		case SDLK_l:
			((RT_PointLight*)light_list.at(0))->position = glm::vec3(0.f, 0.f, 0.f);
			//light_list[1]->position = calcRandPos(LO0x, LO0y, LO0z, HI0x, HI0y, HI0z);
			break;
		}
		camera.Update(glm::vec3(0.f, 0.f, -1.f));
		
		//return;
	}

	/*float x1 ;float x2;
	float y1 ;float y2;
	//const Uint8* keystate = SDL_GetKeyboardState(NULL);
	if (event.type == SDL_MOUSEBUTTONDOWN) {
		left_click = true;
		x1 = event.motion.x;
		y1 = event.motion.y;
	}
	if (event.type == SDL_MOUSEMOTION) {
		//if (event.key.keysym.sym==SDLK_LCTRL) {
		if (left_click) {
			x2 = event.motion.x;
			y2 = event.motion.y;
		}
			//std::cout << "moving mouse x: "<< event.motion.xrel<<" y: "<<event.motion.yrel <<std::endl
		//}
	}
	if (event.type == SDL_MOUSEBUTTONUP) {
		left_click = false;
		camera.camera_position[0] += 0.0001*((x1 - x2));	//	camera.camera_position[0] += 0.001*event.motion.xrel;
		camera.camera_position[1] += 0.0001*((y1 - y2));	//	camera.camera_position[1] += 0.001*event.motion.yrel;
		camera.Update(glm::vec3(0.f, 0.f, 0.1f));
	}*/
	if (event.type == SDL_QUIT)
		quit = 1;
	if (event.type == SDL_KEYUP) {
		switch (event.key.keysym.sym) {
			case SDLK_ESCAPE:
				quit = 1;
		}
	}
}

int main(int argc, char* argv[])
{
	SDL_Window* main_window;
	SDL_Event event;
	SDL_SetRelativeMouseMode(SDL_FALSE);
	float width_step = COORDS_FLOAT_WIDTH / (float)WIDTH;
	float height_step = COORDS_FLOAT_HEIGHT / (float)HEIGHT;

	///CAMERA
	Camera camera = Camera(glm::vec3(-1.60f,1.5f, 0.0f), glm::vec3(0.f,0.f,-1.f), 30.f, (float)WIDTH/ (float)HEIGHT);

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
	CreateWall();
	//CreatePointLight(glm::vec3(-2.f,-1.25f, 2.f), 3.33f, glm::vec3(255, 136, 21));
	
	CreateGlobalLight(glm::vec3(0.f, 0.f, -1.f), 2.f, glm::vec3(255, 255, 255));
	CreatePointLight(glm::vec3(2.f, 1.25f, 2.f), 10.33f, glm::vec3(13, 244, 255));
	CreatePointLight(glm::vec3(-2.25f, 1.25f, 2.f), 5.33, glm::vec3(255, 136, 21));

	int triangle_count;
	int i;
	float view_x;
	float view_y;

	SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, frame_buffer);
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
	SDL_RenderClear(renderer);
	SDL_RenderCopy(renderer, texture, NULL, NULL);
	SDL_RenderPresent(renderer);

	Ray *primary_ray = new Ray();
	float min_dist = inf;
	
	glm::vec3 PHit; glm::vec3 NHit;
	glm::vec3 closest_PHit;

	#ifdef SCREEN_SPACE_SUBSAMPLE
	char mod = 0;
	#endif
	while (!quit)
	{

#ifdef PROFILING
		auto start = std::chrono::high_resolution_clock::now();
#endif
		for (int y = 0; y < HEIGHT; ++y) {
			while (SDL_PollEvent(&event)) {
				MovePolling(event, camera);
			}

#ifdef SCREEN_SPACE_SUBSAMPLE
			//mod ^= 1;
			if (y%SCREEN_SPACE_SUBSAMPLE)continue;
			mod ^= 1;
#endif
			for (int x = 0; x < WIDTH; ++x) {
#ifdef SCREEN_SPACE_SUBSAMPLE
				if ((x + mod) % SCREEN_SPACE_SUBSAMPLE)continue;
#endif
				setRGBAPixel(frame_buffer, x, y, glm::u8vec4(0, 0, 0, 255));

				view_x = (-1.f + (width_step*(float)x)) * camera.scale * camera.aspect_ratio;
				view_y = (1.f - (height_step*(float)y)) * camera.scale;

				min_dist = inf;
				///For every mesh stored
				__int64 mesh_iter_index = -1;
				int mesh_triangle_index = -1;
				float prev_D = 0;
				for (auto mesh = mesh_list.begin(); mesh != mesh_list.end(); ++mesh)
				{
					float PHit_dist = inf;
					//glm::vec3 NHit;
					triangle_count = (*mesh)->getTriangleCount();
					for (i = 0; i < triangle_count; ++i) {///For every triangle of the mesh
						primary_ray = new Ray();

						(Ray::calcRayPerspectiveDirection(primary_ray,
							view_x,
							view_y,
							1.0,
							CAM_NEAR_PLANE, camera));

						//if (x == 161 && y == 120 && i == 0) {
						//	camera.Update(glm::vec3(0.f, 0.f, -1.f));
						//}
						 //float w = 1-u-v
						float u; float v;
						if (RT_Mesh::intersectTriangleMT(true, (*mesh)->getTriangle(i), (*mesh)->isSingleSided(), primary_ray, PHit, NHit, PHit_dist, u, v, min_dist)) {
							mesh_iter_index = mesh - mesh_list.begin();
							mesh_triangle_index = i;
							min_dist = PHit_dist;
							closest_PHit = PHit;
						}
					}
				}
				///IF HIT CALC LIGHTs
				if (!(mesh_iter_index == -1 || mesh_triangle_index == -1)) {
					PHit = closest_PHit;
					//glm::vec3 NHit;
					//setRGBAPixel(frame_buffer, x, y, 0, 255, 0, 255);
					Ray *shadow_ray = new Ray();
					bool is_lit = true;
					RT_Mesh *hit_mesh = *(mesh_list.begin() + mesh_iter_index);
					hit_mesh->getTriangle(mesh_triangle_index);
					//for each light cast shadow ray

					///TODO GET OBJECT COLOR
					//glm::u8vec4 pixel_color = glm::vec3(mesh->color[0], mesh->color[1], mesh->color[2]) * glm::vec3(AMBIENT_LIGHT_STRENGHT);
					glm::f32vec3 pixel_color = glm::f32vec4(0);/*glm::f32vec3(hit_mesh->color) * glm::f32vec3(AMBIENT_LIGHT_STRENGHT, AMBIENT_LIGHT_STRENGHT, AMBIENT_LIGHT_STRENGHT);*/
					//from this point up I should have all hit object properties

					///so far everything will be diffuse
					//TODO other materials switch here!
					for (auto light = light_list.begin(); light != light_list.end(); ++light) {
						//shadow_ray->direction = Ray::calcRayDirection(closest_PHit,(*light)->position);
						//shadow_ray->origin = closest_PHit;//-primary_ray->hit_normal*0.05f;
						//shadow_ray->prev_D = primary_ray->prev_D;///COMMENT THIS OUT EVERYWHERE
						glm::f32vec3 hit_color;
						is_lit = true;
						OUT glm::vec3 light_intensity; OUT glm::vec3 light_direction; OUT float light_distance;
						(*light)->shine(light_intensity, light_distance, light_direction, closest_PHit);
						//NHit, shadow_ray->direction)
						//glm::dot/*(NHit,shadow_ray->direction)*/
						//pro diffuse
						hit_color = glm::clamp((hit_mesh->albedo / glm::f32vec3(M_PI))*light_intensity*std::max(0.f, glm::dot(NHit, -light_direction)), 0.f, FLT_MAX);
						shadow_ray->direction = -light_direction;
						shadow_ray->origin = closest_PHit;// +NHit * 0.0005f;
						shadow_ray->prev_D = primary_ray->prev_D;///COMMENT THIS OUT EVERYWHERE
						////NIZE ZABALIT DO METODY trace
						//we won't intersect object further than this distance of light and the last hit
						//min_dist = RT_Mesh::getDistanceFromOrigin(shadow_ray->direction, light_distance);
						min_dist = inf;
						//for each object 
						for (auto mesh = mesh_list.begin(); mesh != mesh_list.end(); ++mesh) {

							if (!is_lit) { break; }
							triangle_count = (*mesh)->getTriangleCount();//std::cout << std::to_string(triangle_count) << "\n";
							for (i = 0; i < triangle_count; ++i) {///For every triangle of the mesh

								float PHit_dist;
								glm::vec3 NHit_shadow;
								float u; float v;
								if (RT_Mesh::intersectTriangleMT(false, (*mesh)->getTriangle(i), false, shadow_ray, PHit, NHit_shadow, PHit_dist, u, v, min_dist)) {

									is_lit = false;

									break;
								}
								else {
									//is_lit = true;
								}

							}// end for each triangle of mesh
						}//end for each mesh in the scene


						///!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
						///!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
						//is_lit = false;
						///!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
						///!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!


						if (is_lit) {
							//std::cout << "OSVETLUJI PIXEL" << std::endl;
							/// !!! IMPLICIT CAST TO UINT FROM FLOAT !!!
							pixel_color = pixel_color + glm::f32vec3(hit_mesh->color)*hit_color, 0.f, FLT_MAX;
						}
					}//end for each light in the scene
					setRGBAPixel(frame_buffer, x, y, glm::u8vec3(pixel_color));
				}
			}//end for each 
#define FINAL_RENDER
#ifdef FINAL_RENDER
		}
#endif
		SDL_LockSurface(frame_buffer);
		SDL_UpdateTexture(texture, NULL, frame_buffer->pixels, WIDTH * sizeof(Uint32));//
		SDL_UnlockSurface(frame_buffer);
		SDL_RenderCopy(renderer, texture, NULL, NULL);
		SDL_RenderPresent(renderer);
#ifndef FINAL_RENDER
	}
#endif
#ifdef PROFILING
		auto elapsed = std::chrono::high_resolution_clock::now() - start;
		long long microseconds = std::chrono::duration_cast<std::chrono::microseconds>(elapsed).count();
		std::cout << "Frametime: " << std::fixed<<std::setprecision(2)<< (float)microseconds/1000000.f<<"us";
		std::cout << "; " << std::fixed << std::setprecision(2) << 1000000.f/(float)microseconds << "	fps" << std::endl;
		std::flush(std::cout);
#endif

	}
	app_exit(0, texture, renderer, frame_buffer, main_window);
	return 0;
}

