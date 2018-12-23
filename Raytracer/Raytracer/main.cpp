/*#ifdef _DEBUG
#define _CRTDBG_MAP_ALLOC  
#include <crtdbg.h>  
#define DEBUG_NEW new(_NORMAL_BLOCK, __FILE__, __LINE__)
#define new DEBUG_NEW
#endif*/

#include <iostream>
#include <iomanip>
#include <string>
#include <vector>
#include <chrono>
#include <atomic>
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
#include "Model.h"
#include "defines.h"

//Enable external gpu on a laptop supporting nvidia optimus
/*#include <Windows.h>
extern "C" {
	_declspec(dllexport) DWORD NvOptimusEnablement = 0x00000001;
}*/

#ifdef PROFILING
static std::atomic<unsigned long long> triangle_intersection_count = 0;
static std::atomic<unsigned long long> triangle_test_count = 0;
static std::atomic<unsigned long long> primary_rays = 0;
static std::atomic<unsigned long long> box_test_count = 0;
#endif

///TODO
//#define GAMMA 2.2f
//https://developer.nvidia.com/gpugems/GPUGems3/gpugems3_ch24.html
//http://renderwonk.com/blog/index.php/archive/adventures-with-gamma-correct-rendering/

// mark functions returning via reference
#define OUT

// convert  <0.f,1.f> to <0,255>
#define F2U(float_x) (uint8_t)round(float_x * 255.0f)
#define F32vec2U8vec(float_vec) glm::u8vec3(F2U(float_vec[0]),F2U(float_vec[1]),F2U(float_vec[2]))
#define U2F(uint_x) (float)uint_x/255.0f
#define U8vec2F32vec(uint_vec) glm::f32vec3(U2F(uint_vec[0]),U2F(uint_vec[1]),U2F(uint_vec[2]))

bool quit = false;

//const float toRadians = glm::pi<float>() / 180.0f;

const float LO0x = -2;
const float LO0y = -2;
const float LO0z = -2;
const float HI0x = 2;
const float HI0y = 2;
const float HI0z = 2;

glm::f32vec3 sky_color_actual;

std::vector<RT_Mesh*> mesh_list;
std::vector<RT_Light*> light_list;
std::vector<RT_Light*> light_list_off;

bool global_light_on = true;

SDL_Surface* CreateRGBImage(int width, int height) {
	return SDL_CreateRGBSurface(0, width, height, 32, (Uint32)0xff<<24, (Uint32)0xff<<16, (Uint32)0xff<<8, (Uint32)0xff);
}

void setRGBAPixel(SDL_Surface* rendered_image,int x, int y, glm::u8vec3 rgba) {
	//rendered_image->pixels[x + 640 * y] = 
	//unsigned char* pixels = (unsigned char*)rendered_image->pixels;
	((unsigned char*)rendered_image->pixels)[4 * (y*rendered_image->w + x) + 0] = rgba[2];//blue
	((unsigned char*)rendered_image->pixels)[4 * (y*rendered_image->w + x) + 1] = rgba[1];//green
	((unsigned char*)rendered_image->pixels)[4 * (y*rendered_image->w + x) + 2] = rgba[0];//red
	((unsigned char*)rendered_image->pixels)[4 * (y*rendered_image->w + x) + 3] = glm::u8vec1(255)[0];//rgba[3];//alpha
	//free(pixels);
}

glm::u8vec3 getRGBAPixel(SDL_Surface* rendered_image,int x,int y) {
	return glm::u8vec3(((unsigned char*)rendered_image->pixels)[4 * (y*rendered_image->w + x) + 2], ((unsigned char*)rendered_image->pixels)[4 * (y*rendered_image->w + x) + 1], ((unsigned char*)rendered_image->pixels)[4 * (y*rendered_image->w + x) + 0]);
}

void CreatePointLight(glm::vec3 pos, float intensity, glm::vec3 color) {
	light_list.push_back(new RT_PointLight(pos, intensity, color));
}

void CreateGlobalLight(glm::vec3 direction, float intensity, glm::vec3 color) {
	light_list.push_back(new RT_DistantLight(direction, intensity/*/GLOBAL_LIGHT_DIVISOR*/, color));
}

void updateSkyColor() {
	sky_color_actual = glm::f32vec3(0.f);
	for (auto light = light_list.begin(); light != light_list.end(); ++light) {
		if ((*light)->getType() == RT_Light::distant) {
			sky_color_actual += glm::clamp(const_sky_color*AMBIENT_LIGHT*2.f + const_sky_color * (*light)->intensity/4.f, 0.f, 1.f);
		}
	}
}

//glm::u8vec3 quantize(glm::f32vec3 fcolor) {
//	glm::f32vec3 qcolor;
//
//	return glm::u8vec3(qcolor);
//}
/*
void CreateWall(float w, float h,float z, glm::f32vec3 color, RT_Mesh::MATERIAL_TYPE type,bool frontfacing) {
	/*float vertices[] = {
		//zadni stena
		-w/2.f, -h/2.f, z,
		w/2.f, -h/2.f, z,
		w/2.f, h/2.f, z,
		-w/2.f, h/2.f, z
	};
	RT_Mesh* plane;
	RT_Mesh::Vertex vertices[4];
	vertices[0].position = glm::f32vec3(-w / 2.f, -h / 2.f, z);
	vertices[1].position = glm::f32vec3(w / 2.f, -h / 2.f, z);
	vertices[2].position = glm::f32vec3(w / 2.f, h / 2.f, z);
	vertices[3].position = glm::f32vec3(-w / 2.f, h / 2.f, z);

	if (frontfacing) {
		unsigned int indices[6]={
			//zadni stena
			0,1,2,
			0,2,3
		};
		plane = new RT_Mesh(vertices, indices, *(&vertices + 1) - vertices, *(&indices + 1) - indices, false, color, 0.2f, type);
	}
	else {
		unsigned int indices[6]= {
			//zadni stena
			2,1,0,
			3,2,0
		};
		plane = new RT_Mesh(vertices, indices, *(&vertices + 1) - vertices, *(&indices + 1) - indices, false, color, 0.2f, type);
	}
	mesh_list.push_back(plane);
}*/
/*
void CreateBox() {

	/*float vertices[] = {
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
	};* /

	struct RT_Mesh::Vertex vertices[8];
	vertices[0].position = glm::f32vec3(-1.f, -1.f, -2.5f);
	vertices[1].position = glm::f32vec3(1.f, -1.f, -2.5f);
	vertices[2].position = glm::f32vec3(1.f, 1.f, -2.5f);
	vertices[3].position = glm::f32vec3(-1.f, 1.f, -2.5f);

	vertices[4].position = glm::f32vec3(-1.f, -1.f, -5.f);
	vertices[5].position = glm::f32vec3(1.f, -1.f, -5.f);
	vertices[6].position = glm::f32vec3(1.f, 1.f, -5.f);
	vertices[7].position = glm::f32vec3(-1.f, 1.f, -5.f);

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

	glm::f32vec3 color = glm::f32vec3(U2F(255),U2F(127),U2F(127));

	RT_Mesh* box = new RT_Mesh(vertices, indices, *(&vertices+1)-vertices, *(&indices + 1) - indices, false, color,0.23f/M_PI, RT_Mesh::DIFFUSE);
	mesh_list.push_back(box);
}*/
/*
void CreateBox2() {

	/*float vertices[] = {
		//predni stena
		-4.f, -1.5f, -2.f,
		-2.f, -1.5f, -2.f,
		-2.f, .5f, -2.f,
		-4.f, .5f, -2.f,
		//zadni stena
		-4.f, -1.5f, -4.5f,
		-2.f, -1.5f, -4.5f,
		-2.f, .5f, -4.5f,
		-4.f, .5f, -4.5f
	};* /
	RT_Mesh::Vertex vertices[8];
	vertices[0].position = glm::f32vec3(-4.f, -1.5f, -2.f);
	vertices[1].position = glm::f32vec3(-2.f, -1.5f, -2.f);
	vertices[2].position = glm::f32vec3(-2.f, .5f, -2.f);
	vertices[3].position = glm::f32vec3(-4.f, .5f, -2.f);

	vertices[4].position = glm::f32vec3(-4.f, -1.5f, -4.5f);
	vertices[5].position = glm::f32vec3(-2.f, -1.5f, -4.5f);
	vertices[6].position = glm::f32vec3(-2.f, .5f, -4.5f);
	vertices[7].position = glm::f32vec3(-4.f, .5f, -4.5f);


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

	glm::f32vec3 color = glm::f32vec3(U2F(127), U2F(136), U2F(255));

	RT_Mesh* box = new RT_Mesh(vertices, indices, *(&vertices + 1) - vertices, *(&indices + 1) - indices, false, color, 0.17f / M_PI, RT_Mesh::DIFFUSE);		//indices vertices length pointer arithmetic 
	//box->CreateMesh(vertices, indices, *(&vertices + 1) - vertices, *(&indices + 1) - indices, false, color, 0.17f / M_PI,RT_Mesh::DIFFUSE);
	mesh_list.push_back(box);
}
*/

int app_exit(int return_code,SDL_Texture* texture, SDL_Renderer* renderer, SDL_Surface* frame_buffer, SDL_Window* main_window)
{
	std::cout << "Ragequit();" << std::endl;
	SDL_DestroyTexture(texture);
	SDL_FreeSurface(frame_buffer);
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(main_window);
	SDL_Quit();
	
	//delete texture;
	///Free/delete objects stored in vectors
	//int len = mesh_list.size();
	for (int i = mesh_list.size() - 1; i >=0; --i) {
		//mesh_list[i]->ClearMesh;
		delete(mesh_list.at(i));
	}
	//len = light_list.size();
	for (int i = light_list.size() - 1; i >= 0; --i) {
		delete(light_list.at(i));
	}
	///Free the vectors by redeclaring
	mesh_list.clear(); light_list.clear();

#ifdef _DEBUG
	_CrtDumpMemoryLeaks();
#endif
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
			camera.camera_position[2] -= 0.1f;
			break;
		case SDLK_s:
			camera.camera_position[2] += 0.1f;
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
		case SDLK_g:
			if (global_light_on) {
				for (auto light = light_list.begin(); light != light_list.end(); /*++light*/) {
					if ((*light)->getType() == RT_Light::distant) {
						light_list_off.push_back(*light);
						light_list.erase(light);
					}
					else light++;
				}
			}
			else {
				for (auto light = light_list_off.begin(); light != light_list_off.end(); /*++light*/) {
					if ((*light)->getType() == RT_Light::distant) {
						light_list.push_back(*light);
						light_list_off.erase(light);
					}
					else light++;
				}
			}
			global_light_on ^= 1;
			updateSkyColor();
			break;
		}
		camera.Update(glm::vec3(0.f, 0.f, -1.f));
		
		//return;
	}

	if (event.type == SDL_QUIT)
		quit = 1;
	if (event.type == SDL_KEYUP) {
		switch (event.key.keysym.sym) {
			case SDLK_ESCAPE:
				quit = 1;
		}
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
}

/*
* return pixel_color
*/
glm::f32vec3 RayTrace(const glm::vec3 &origin, const glm::vec3 &ray_dir, const uint8_t & depth = 0) {

	glm::f32vec3 pixel_color = sky_color_actual;
	if (depth > MAX_DEPTH) {
		return pixel_color;
	}

	Ray *primary_ray;
	Ray *shadow_ray; bool is_lit;
	float min_dist = inf;

	glm::vec3 PHit; glm::vec3 NHit;
	glm::vec3 closest_PHit;
	
	

	__int64 mesh_iter_index;
	int mesh_triangle_index;
	float prev_D;
	uint32_t triangle_count;
	uint32_t i;
	///For every mesh stored
	min_dist = inf;
	mesh_iter_index = -1;
	mesh_triangle_index = -1;
	prev_D = 0;

	primary_ray = new Ray();
	primary_ray->direction = ray_dir;
	primary_ray->origin = origin;
#ifdef BBAccel
	primary_ray->precomputeValues();
#endif
	float u; float v;
	glm::vec3 N_hit;
	for (auto mesh = mesh_list.begin(); mesh != mesh_list.end(); ++mesh)
	{
#ifdef BBAccel		
		std::atomic_fetch_add(&box_test_count,1);
		OUT float t; //distance of box intersection;
		if (!primary_ray->intersectBB((*mesh)->boundary_points,t)) {
			continue;
		}
		if (t < 0)break;
#endif
		
		float PHit_dist = inf;
		//if(mesh->material == RT_Mesh::DIFFUSE)
		//glm::vec3 NHit;
		triangle_count = (*mesh)->getTriangleCount();
		for (i = 0; i < triangle_count; ++i) {///For every triangle of the mesh
			//if (x == 161 && y == 120 && i == 0) {
			//	camera.Update(glm::vec3(0.f, 0.f, -1.f));
			//}
			//float w = 1-u-v
			float u_prim; float v_prim;
			if (primary_ray->intersectTriangleMT(true, (*mesh)->getTriangle(i), (*mesh)->isSingleSided(), PHit, NHit, PHit_dist, u_prim, v_prim, min_dist)) {
				N_hit = NHit;
				u = u_prim; v = v_prim;
				mesh_iter_index = mesh - mesh_list.begin();
				mesh_triangle_index = i;
				min_dist = PHit_dist;
				closest_PHit = PHit;
			}
			/*else {
				delete(primary_ray);
			}*/

		}
	}
	///IF HIT DECIDE MATERIAL
	
	if (!(mesh_iter_index == -1 || mesh_triangle_index == -1)) {
		PHit = closest_PHit;
		float PHit_dist;
		//float u_shad; float v_shad;
		glm::vec3 NHit_shadow;

		RT_Mesh *hit_mesh = *(mesh_list.begin() + mesh_iter_index);

		pixel_color = glm::f32vec3(0.f);

		switch (hit_mesh->_material_type)
		{
		case RT_Mesh::MIRROR:
			glm::f32vec3 hit_color = glm::f32vec3(0);
			glm::vec3 direction = primary_ray->direction;
			Ray::calcReflectedDirection(NHit, direction);
			min_dist = inf;
#ifdef REFLECTION_BIAS
			hit_color += 0.8f * RayTrace(PHit + NHit * HIT_BIAS,direction,depth+1);
#else
			hit_color += 0.8f * RayTrace(PHit, direction, depth + 1);
#endif
			pixel_color = glm::clamp(pixel_color + hit_color, 0.f, 1.f);
			break;

		case RT_Mesh::PHONG:
			shadow_ray = new Ray();
			is_lit = true;
			
			glm::f32vec3 d=glm::f32vec3(0); glm::f32vec3 s=glm::f32vec3(0);
			Vertex hit_triangle[3];
			std::memcpy( hit_triangle, hit_mesh->getTriangle(mesh_triangle_index),sizeof(Vertex)*3 );
			/*Vertex v0 = hit_triangle[0];
			Vertex v1 = hit_triangle[1];
			Vertex v2 = hit_triangle[2];*/
			//for each light cast shadow ray
			/// CALC LIGHTs
			glm::u8vec3 texture_diffuse_color;
			for (auto light = light_list.begin(); light != light_list.end(); ++light) {
				//hit_color = glm::f32vec3(0);
				is_lit = true;
				OUT glm::vec3 light_intensity; OUT glm::vec3 light_direction; OUT float light_distance;
				(*light)->shine(light_intensity, light_distance, light_direction, closest_PHit);
				//pro diffuse
				//hit_color = (hit_mesh->albedo)*light_intensity*glm::f32vec3(std::max(0.f, glm::dot(NHit, -light_direction)));
				shadow_ray->direction = -light_direction;
				shadow_ray->origin = closest_PHit +NHit*HIT_BIAS;
				shadow_ray->prev_D = primary_ray->prev_D;///COMMENT THIS OUT EVERYWHERE
#ifdef BBAccel
				shadow_ray->precomputeValues();
#endif
				//for each object
				for (auto mesh_itr = mesh_list.begin(); mesh_itr != mesh_list.end(); ++mesh_itr)
				{
					RT_Mesh* mesh = (*mesh_itr);
#ifdef BBAccel	
					std::atomic_fetch_add(&box_test_count, 1);
					OUT float t; //distance of box intersection;
					if (!shadow_ray->intersectBB(mesh->boundary_points, t)) {
						continue;
					}
#endif
					if (!is_lit) { break; }
					triangle_count = mesh->getTriangleCount();//std::cout << std::to_string(triangle_count) << "\n";
					///For every triangle of the mesh
					for (i = 0; i < triangle_count; ++i) {
						NHit_shadow;
						float u_shadow; float v_shadow;
						//increment triangle tested counter
						std::atomic_fetch_add(&triangle_test_count, 1);
						if (shadow_ray->intersectTriangleMT(false, mesh->getTriangle(i), false, PHit, NHit_shadow, PHit_dist, u_shadow, v_shadow, light_distance-HIT_BIAS)) {
							//increment triangle intersected counter
							std::atomic_fetch_add(&triangle_intersection_count, 1);
							is_lit = false;
							break;
						}
						else {
							//is_lit = true;
						}
					}// end for each triangle of mesh
				}//end for each mesh in the scene
				/*if (is_lit) {*/
#ifndef GAMMA
					glm::vec2 tex_coords = glm::vec2(-1.f, -1.f);
					Texture texture; glm::vec3 N;
					if (!hit_mesh->GetTextures().empty())
					{
						for (auto texItr = hit_mesh->GetTextures().begin(); texItr != hit_mesh->GetTextures().end(); ++texItr)
						{
							// TODO - should be strcmp
							if ((*texItr).type == "texture_diffuse")
							{
								texture = (*texItr);
							}
						}

						GetHitProperties(hit_triangle[0], hit_triangle[1], hit_triangle[2], u, v, texture.height, texture.width, N, tex_coords);

						uint32_t texelIndex = 3 * (glm::clamp((int)tex_coords.x, 0, texture.width - 1) + (texture.width)*glm::clamp((int)tex_coords.y, 0, texture.height - 1));

						texture_diffuse_color = glm::u8vec3(texture.data[0 + texelIndex],
							texture.data[1 + texelIndex],
							texture.data[2 + texelIndex]
						);
					}
					else
					{
						GetHitProperties(hit_triangle[0], hit_triangle[1], hit_triangle[2], u, v, N);
					}
					glm::vec3 ref_dir = light_direction;
#ifdef SMOOTH_SHADING
					d+= (float)is_lit * hit_mesh->_albedo * light_intensity * glm::f32vec3(std::max(0.f, glm::dot(N, -light_direction)));
					Ray::calcReflectedDirection(N, ref_dir);
#else
					d += (float)is_lit * hit_mesh->_albedo * light_intensity * glm::f32vec3(std::max(0.f, glm::dot(N_hit, -light_direction)));
					Ray::calcReflectedDirection(N_hit, ref_dir);
#endif
					s+= (float)is_lit * light_intensity * std::pow(std::max(0.f, glm::dot(ref_dir, -ray_dir)), hit_mesh->_material.shininess);		
#else
					pixel_color = glm::clamp(glm::pow(pixel_color + hit_mesh->color*hit_color, glm::f32vec3(GAMMA)), 0.f, 1.f);
#endif
			}//end for each light in the scene

			if (hit_mesh->GetTextures().empty())
			{
				pixel_color = glm::clamp((1.0f*hit_mesh->_material.emissive_color + d * hit_mesh->_material.diffuse_color + s * hit_mesh->_material.specluar_color + hit_mesh->_material.ambient_color*AMBIENT_LIGHT), 0.f, 1.f);
			}
			else
			{
				pixel_color = glm::clamp((1.0f*hit_mesh->_material.emissive_color + d * U8vec2F32vec(texture_diffuse_color) + s * U8vec2F32vec(texture_diffuse_color) + U8vec2F32vec(texture_diffuse_color)*AMBIENT_LIGHT), 0.f, 1.f);
			}
			delete(shadow_ray);
			break;

		case RT_Mesh::DIFFUSE:
		default:
			shadow_ray = new Ray();
			is_lit = true;
			
			//hit_mesh->getTriangle(mesh_triangle_index);
			//for each light cast shadow ray
			/// CALC LIGHTs
			for (auto light = light_list.begin(); light != light_list.end(); ++light) {
				//hit_color = glm::f32vec3(0);
				is_lit = true;
				OUT glm::vec3 light_intensity; OUT glm::vec3 light_direction; OUT float light_distance;
				(*light)->shine(light_intensity, light_distance, light_direction, closest_PHit);

				shadow_ray->direction = -light_direction;
				shadow_ray->origin = closest_PHit +NHit * HIT_BIAS;
				shadow_ray->prev_D = primary_ray->prev_D;///COMMENT THIS OUT EVERYWHERE
#ifdef BBAccel
				shadow_ray->precomputeValues();
#endif
				////ENCAPSULATE FOLLOWIN'?
				//we won't intersect object further than this distance of light and the last hit
				min_dist = RT_Mesh::getDistanceFromOrigin(shadow_ray->direction, -light_direction);

				//for each object
				for (auto mesh = mesh_list.begin(); mesh != mesh_list.end(); ++mesh) {
#ifdef BBAccel
					std::atomic_fetch_add(&box_test_count, 1);
					OUT float t; //distance of box intersection;
					if (!shadow_ray->intersectBB((*mesh)->boundary_points, t)) {
						continue;
					}
					if (t < 0)break;
#endif
					if (!is_lit) { break; }
					triangle_count = (*mesh)->getTriangleCount();//std::cout << std::to_string(triangle_count) << "\n";
					///For every triangle of the mesh
					for (i = 0; i < triangle_count; ++i) {
						NHit_shadow;
						//float u; float v;
						//increment triangle tested counter
						std::atomic_fetch_add(&triangle_test_count,1);
						if (shadow_ray->intersectTriangleMT(false, (*mesh)->getTriangle(i), false, PHit, NHit_shadow, PHit_dist, u, v, min_dist)) {
							//increment triangle intersected counter
							std::atomic_fetch_add(&triangle_intersection_count,1);
							is_lit = false;
							break;
						}
						else {
							//is_lit = true;
						}
					}// end for each triangle of mesh
				}//end for each mesh in the scene
				//if (is_lit) {
								//pro diffuse
				hit_color = (hit_mesh->_albedo)*light_intensity*glm::f32vec3(std::max(0.f, glm::dot(NHit, -light_direction)));

				pixel_color = pixel_color + (float)is_lit*hit_mesh->_material.diffuse_color*hit_color;

					//return 1;
				//}
				//else {
					//pixel_color = pixel_color;
				//}
			}//end for each light in the scene
			pixel_color = glm::clamp(pixel_color + hit_mesh->_material.ambient_color *AMBIENT_LIGHT,0.f,1.f);
			delete(shadow_ray);
			break;
		}
		
	}
	delete(primary_ray);		
	return pixel_color;
}//end for each mesh SHADOW RAYS


int main(int argc, char* argv[])
{/*
#ifdef _DEBUG
	_crtBreakAlloc = -1;
#endif*/
	SDL_Window* main_window;
	SDL_Event event;
	SDL_SetRelativeMouseMode(SDL_FALSE);
	float width_step = COORDS_FLOAT_WIDTH / (float)WIDTH;
	float height_step = COORDS_FLOAT_HEIGHT / (float)HEIGHT;

	///CAMERA
	Camera camera = Camera(glm::vec3(0.f,0.2f,1.f), glm::vec3(0.f,0.f,-1.f), 50.f, (float)WIDTH/ (float)HEIGHT);
	
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
	
	std::string modelPath;
	if (argc == 2)
	{
		modelPath = std::string(argv[1]);
	}
	else
	{
		char currentDir[FILENAME_MAX];
		GetCurrentDir(currentDir, FILENAME_MAX);
		modelPath = std::string(currentDir).append("/").append(DEFAULT_MODEL);
	}

	LoadScene(modelPath, mesh_list);

	CreatePointLight(glm::vec3(.5f, 2.f, 2.f), 100.f, glm::f32vec3(U2F(64), U2F(134), U2F(244)));
	CreatePointLight(glm::vec3(-0.5f, -2.0f, 2.f), 100.f, glm::f32vec3(U2F(244), U2F(174), U2F(66)));
	CreateGlobalLight(glm::vec3(0.5f, -6.f, -1.f), global_light_intensity, glm::f32vec3(U2F(255), U2F(255), U2F(255)));

	updateSkyColor();

	int triangle_count;
	float view_x;
	float view_y;

	SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, frame_buffer);
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
	SDL_RenderClear(renderer);
	SDL_RenderCopy(renderer, texture, NULL, NULL);
	SDL_RenderPresent(renderer);
	glm::f32vec3 pixel_color;

#ifdef PROFILING
	std::chrono::steady_clock::duration elapsed;
	std::chrono::steady_clock::time_point start;
	unsigned long long microseconds;
#endif
	#ifdef SCREEN_SPACE_SUBSAMPLE
	char mod = 0;
	#endif
	while (!quit)
	{

#ifdef PROFILING
		start = std::chrono::high_resolution_clock::now();
		std::atomic_fetch_and(&primary_rays,0);
		std::atomic_fetch_and(&triangle_intersection_count, 0);
		std::atomic_fetch_and(&triangle_test_count, 0);
		std::atomic_fetch_and(&box_test_count, 0);
#endif
		for (int y = 0; y < HEIGHT; ++y) {
			while (SDL_PollEvent(&event)) {
				MovePolling(event, camera);
			}
#ifdef SCREEN_SPACE_SUBSAMPLE
			//mod ^= 1;
			if (y%SCREEN_SPACE_SUBSAMPLE){
				for (int x = 0; x < WIDTH; ++x) {
					if (SCREEN_SPACE_SUBSAMPLE == 1) {
						setRGBAPixel(frame_buffer, x, y, getRGBAPixel(frame_buffer, x, y - 1));
					}
					else {
						if (y%SCREEN_SPACE_SUBSAMPLE <= (SCREEN_SPACE_SUBSAMPLE / 2))
						{
							if (y > SCREEN_SPACE_SUBSAMPLE) {
								setRGBAPixel(frame_buffer,x,y-(SCREEN_SPACE_SUBSAMPLE/2+1), getRGBAPixel(frame_buffer, x, y - 1));
							}
							setRGBAPixel(frame_buffer, x, y, getRGBAPixel(frame_buffer, x, y - 1));
						}
						/*else if((y>SCREEN_SPACE_SUBSAMPLE))
						{
						//	setRGBAPixel(frame_buffer, x, y, getRGBAPixel(frame_buffer, x, y - (2*SCREEN_SPACE_SUBSAMPLE-1)));
						}*/

					}
				}
				continue;
			}
			mod ^= 1;
#endif
			for (int x = 0; x < WIDTH; ++x) {
#ifdef SCREEN_SPACE_SUBSAMPLE

				if ((x + mod) % SCREEN_SPACE_SUBSAMPLE) {	
					//setRGBAPixel(frame_buffer, x, y, F32vec2U8vec(pixel_color));
					if (SCREEN_SPACE_SUBSAMPLE == 1) {
						setRGBAPixel(frame_buffer, x, y, F32vec2U8vec(pixel_color));
					}
					else {
						if (x%SCREEN_SPACE_SUBSAMPLE <= (SCREEN_SPACE_SUBSAMPLE / 2))
						{
							if (x > SCREEN_SPACE_SUBSAMPLE) {
								setRGBAPixel(frame_buffer, x - (SCREEN_SPACE_SUBSAMPLE / 2 + 1), y, F32vec2U8vec(pixel_color));
							}
							setRGBAPixel(frame_buffer, x, y, F32vec2U8vec(pixel_color));
						}
						/*else if((y>SCREEN_SPACE_SUBSAMPLE))
						{
						//	setRGBAPixel(frame_buffer, x, y, getRGBAPixel(frame_buffer, x, y - (2*SCREEN_SPACE_SUBSAMPLE-1)));
						}*/

					}
					continue; 
				}
				
#endif
#ifdef MSAA	
				glm::f32vec3 cumulative_color = glm::f32vec3(0);
				for (uint8_t sample = 0; sample < msaa_sample_coords.size(); ++sample) {
					view_x = (-1.f + (width_step*((float)x+msaa_sample_coords[sample][0]))) * camera.scale * camera.aspect_ratio;
					view_y = (1.f - (height_step*((float)y + msaa_sample_coords[sample][1]))) * camera.scale;
#else
					view_x = (-1.f + (width_step*(float)x)) * camera.scale * camera.aspect_ratio;
					view_y = (1.f - (height_step*(float)y)) * camera.scale;
#endif
					OUT glm::vec3 origin; OUT glm::vec3 direction;
					(Ray::calcRayPerspectiveDirection(OUT origin, OUT direction, view_x, view_y, CAM_NEAR_PLANE, camera));
					//setRGBAPixel(frame_buffer, x, y, glm::u8vec4(0, 0, 0, 255));
					///raytrace
					std::atomic_fetch_add(&primary_rays, 1);
					
#ifdef MSAA	
					cumulative_color += RayTrace(origin, direction);
				}
			pixel_color = cumulative_color / (float)msaa_sample_coords.size();
#else
				pixel_color = RayTrace(origin, direction);
#endif				
				setRGBAPixel(frame_buffer, x, y, F32vec2U8vec(pixel_color) );

			}//end for each pixel (x)

#define FINAL_RENDER
#ifdef FINAL_RENDER
		}//end for each line y  - whole image processed
#endif
		//SDL_LockSurface(frame_buffer);
		SDL_UpdateTexture(texture, NULL, frame_buffer->pixels, WIDTH * sizeof(Uint32));//
		//SDL_UnlockSurface(frame_buffer);
		SDL_RenderCopy(renderer, texture, NULL, NULL);
		SDL_RenderPresent(renderer);
		
#ifndef FINAL_RENDER
	}
#endif
#ifdef PROFILING
		elapsed = std::chrono::high_resolution_clock::now() - start;
		microseconds = std::chrono::duration_cast<std::chrono::microseconds>(elapsed).count();
		std::cout << "Frametime: " << std::fixed<<std::setprecision(2)<< (float)microseconds/1000000.f<<"s";
		std::cout << "; " << std::fixed << std::setprecision(2) << 1000000.f/(float)microseconds << "	fps" << std::endl;
		std::cout << "Primary rays: " << std::fixed << std::setprecision(2) << primary_rays/1000.f<< "K" << std::endl;
		std::cout << "BBox tests: " << std::fixed << std::setprecision(3) << box_test_count / 1000000.f << "M" << std::endl;
		std::cout << "Triangles intersected: " <<std::fixed<<std::setprecision(3) << triangle_intersection_count/ 1000.f << "K" << std::endl;
		std::cout << "Triangles tested: " << std::fixed <<std::setprecision(3) << triangle_test_count /1000000.f << "M \n" << std::endl;
		std::flush(std::cout);
#endif
#ifdef ONE_FRAME
		break;
#endif
	}
	return app_exit(0, texture, renderer, frame_buffer, main_window);
}

