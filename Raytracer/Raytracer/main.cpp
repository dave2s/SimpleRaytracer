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
#include <thread>
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
#include "Model.h"
#include "defines.h"
#include <glm/gtc/random.hpp>
#include "BVH.h"
#include "AccelerationStructure.h"
#include "BBoxAcceleration.h"
#include "main.h"
#include <mutex>

//Enable external gpu on a laptop supporting nvidia optimus
/*#include <Windows.h>
extern "C" {
	_declspec(dllexport) DWORD NvOptimusEnablement = 0x00000001;
}*/

#ifdef PROFILING
static std::atomic<unsigned long long> triangle_intersection_count = 0;
static std::atomic<unsigned long long> triangle_test_count = 0;
static std::atomic<unsigned long long> primary_rays = 0;
static std::atomic<unsigned char> global_light_on = true;
/*#ifdef BBAccel
static std::atomic<unsigned long long> box_test_count = 0;
#endif*/
#endif

//Borosilicate glass BK7
//float B = 1.5046f;
//float C = 0.00420f;

//Diamond
float B = 2.385f;
float C = 0.0117f;

float global_cauchy_B = B;
float global_cauchy_C = C;

///TODO
//#define GAMMA 2.2f
//https://developer.nvidia.com/gpugems/GPUGems3/gpugems3_ch24.html
//http://renderwonk.com/blog/index.php/archive/adventures-with-gamma-correct-rendering/

bool quit = false;

//const float toRadians = glm::pi<float>() / 180.0f;

const float LO0x = -2;
const float LO0y = -2;
const float LO0z = -2;
const float HI0x = 2;
const float HI0y = 2;
const float HI0z = 2;

glm::f32vec3 sky_color_actual;

std::vector<std::unique_ptr<const RT_Mesh>> mesh_list;
std::vector<RT_Light*> light_list;
std::vector<RT_Light*> light_list_off;

std::vector<int> wavelengths;
int wavelengths_size;

// How many threads can I use
static uint16_t g_thread_count;

// Working threads
static std::atomic_int g_working;
static std::mutex barrier;

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
	/*for (auto& mesh : mesh_list) {
		//mesh_list[i]->ClearMesh;
		//delete(*mesh);

	}*/
	mesh_list.erase(mesh_list.cbegin(), mesh_list.cend());
	//len = light_list.size();
	for (size_t i = light_list.size() - 1; i >= 0; --i) {
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

void resetLightsPositions() {
	for (auto light = light_list.begin(); light != light_list.end(); /*++light*/) {
		if ((*light)->getType() == RT_Light::point) {
			((RT_PointLight*)(*light))->resetPosition();
		}
		light++;
	}
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
				((RT_PointLight*)light_list.at(0))->_position[0] -= .1f;
			break;
		case SDLK_RIGHT:
			if (!light_list.empty())
				((RT_PointLight*)light_list.at(0))->_position[0] += .1f;
			break;
		case SDLK_UP:
			if (!light_list.empty())
				((RT_PointLight*)light_list.at(0))->_position[1] += .1f;
			break;
		case SDLK_DOWN:
			if (!light_list.empty())
				((RT_PointLight*)light_list.at(0))->_position[1] -= .1f;
			break;
		case SDLK_KP_2:
			if (!light_list.empty())
				((RT_PointLight*)light_list.at(0))->_position[2] += .1f;
			break;
		case SDLK_KP_8:
			if (!light_list.empty())
				((RT_PointLight*)light_list.at(0))->_position[2] -= .1f;
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
			resetLightsPositions();
			//light_list[1]->position = calcRandPos(LO0x, LO0y, LO0z, HI0x, HI0y, HI0z);
			break;
		case SDLK_g:
			if (global_light_on) {
				for (auto light = light_list.begin(); light != light_list.end(); /*++light*/) {
					if ((*light)->getType() == RT_Light::distant) {
						light_list_off.push_back(*light);
						light = light_list.erase(light);
					}
					else light++;
				}
			}
			else {
				for (auto light = light_list_off.begin(); light != light_list_off.end(); /*++light*/) {
					if ((*light)->getType() == RT_Light::distant) {
						light_list.push_back(*light);
						light =light_list_off.erase(light);
					}
					else light++;
				}
			}
			std::atomic_fetch_xor(&global_light_on, 1);
			//global_light_on ^= 1;
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


///FOR DEBUGING
inline static void fresnel_debug(const float& ior, float& kr,glm::vec3& dir, glm::vec3& N) {
	float ior_in = 1;
	float ior_out = ior;
	glm::vec3 n = N;
	float cos_in = glm::clamp(-1.f, 1.f, glm::dot(dir, n));

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

float my_rand() {
	return glm::linearRand(0.f, 1.f);
}

void init_wave_info(){
	int step = int((wavelengths_intervals.back() - wavelengths_intervals.front()) / WAVE_SAMPLES);
	int wavelength_max = wavelengths_intervals.back();
	for (int wave = wavelengths_intervals.front()/*+int(step/2.f)*/; wave <= wavelength_max; wave += step ) {
		wavelengths.push_back(wave);
	}
	wavelengths_size = wavelengths.size();
}

/*
* return pixel_color
*/
glm::f32vec3 raytrace(const std::unique_ptr<AccelerationStructure>& accel, const glm::vec3 &origin, const glm::vec3 &ray_dir, const uint8_t & depth = 0, bool monochromatic = false, int wavelength = 0) {
	glm::f32vec3 pixel_color = sky_color_actual;
	if (depth > MAX_DEPTH) {
		return pixel_color;
	}

	Ray *primary_ray;
	Ray *shadow_ray; bool is_lit;
	float min_dist;
	
	__int64 mesh_iter_index;
	float prev_D;
	///For every mesh stored
	min_dist = inf;
	mesh_iter_index = -1;
	prev_D = 0;

	primary_ray = new Ray();
	primary_ray->direction = ray_dir;
	primary_ray->origin = origin;
	primary_ray->isMonochrom = monochromatic;
	primary_ray->wavelength = wavelength;
#if defined (BVH_ACCEL)
	primary_ray->precomputeValues();
#elif defined (BBAccel)
	primary_ray->precomputeValues();
#endif	
	float PHit_dist = inf;
	Ray::Hitinfo info;
	const RT_Mesh* hit_mesh = accel->intersect(primary_ray, PHit_dist, info);
	if (hit_mesh != nullptr) {	
	///IF HIT DECIDE MATERIAL	
		pixel_color = glm::f32vec3(0.f);
		glm::f32vec3 hit_color = glm::f32vec3(0);

		switch (hit_mesh->_material_type)
		{
		case RT_Mesh::MIRROR:
		{
			glm::vec3 direction = primary_ray->direction;
			Ray::calcReflectedDirection(info.NHit, direction);
			min_dist = inf;
#ifdef REFLECTION_BIAS
			hit_color += 0.8f * raytrace(accel,PHit + NHit * HIT_BIAS, direction, depth + 1);
#else
			hit_color += 0.8f * raytrace(accel, info.PHit, direction, depth + 1);
#endif
			pixel_color = glm::clamp(pixel_color + hit_color, 0.f, 1.f);
		}
		break;
		case RT_Mesh::REFRACTION:
		{

				glm::f32vec3 refract_color = glm::f32vec3(0);
				glm::f32vec3 reflect_color = glm::f32vec3(0);

				OUT float kr; min_dist = inf; glm::vec3 bias = primary_ray->hit_normal * HIT_BIAS;

				Ray::fresnel(hit_mesh->_material.ior, kr, primary_ray->direction, primary_ray->hit_normal);

				bool outside = glm::dot(primary_ray->direction, primary_ray->hit_normal);
				// compute refraction if it is not a case of total internal reflection
				if (kr < 1) {
					if (primary_ray->isMonochrom) {
						glm::vec3 refract_dir = glm::normalize(Ray::refract(hit_mesh->_material.ior, primary_ray->direction, primary_ray->hit_normal));
						glm::vec3 refract_orig = outside ? info.PHit - bias : info.PHit + bias;
						refract_color = raytrace(accel,refract_orig, refract_dir, depth + 1,true,primary_ray->wavelength);
					}
					else {

						for (int i = 0; i < wavelengths_size;++i) {		
							glm::vec3 refract_dir = glm::normalize(Ray::refract(Ray::iorFromWavelength(wavelengths[i],global_cauchy_B,global_cauchy_C), primary_ray->direction, primary_ray->hit_normal));
							glm::vec3 refract_orig = outside ? info.PHit - bias : info.PHit + bias;
							refract_color += raytrace(accel,refract_orig, refract_dir, depth + 1, true, wavelengths[i]);
						}
						refract_color /= WAVE_SAMPLES;
					}
				}

				glm::vec3 reflect_dir = primary_ray->direction;
				Ray::calcReflectedDirection(primary_ray->hit_normal, reflect_dir);
				glm::vec3 reflect_orig = outside ? info.PHit + bias : info.PHit - bias;
				reflect_color = raytrace(accel,reflect_orig, reflect_dir, depth + 1);

				hit_color += reflect_color * kr + refract_color * (1 - kr);
				pixel_color = hit_color;
		}
		break;
		case RT_Mesh::PHONG:
		{
			shadow_ray = new Ray();
			//is_lit = true;
			Ray::Hitinfo shadow_info;

			glm::f32vec3 d = glm::f32vec3(0); glm::f32vec3 s = glm::f32vec3(0);
			Vertex hit_triangle[3];
			std::memcpy(hit_triangle, hit_mesh->getTriangle(info.tri_idx), sizeof(Vertex) * 3);
			//for each light cast shadow ray
			/// CALC LIGHTs
			shadow_ray->prev_D = primary_ray->prev_D;///COMMENT THIS OUT EVERYWHERE
			glm::u8vec3 texture_diffuse_color;
			for (auto &light : light_list) {
				is_lit = true;
				OUT glm::vec3 light_intensity; OUT glm::vec3 light_direction; OUT float light_distance;
				light->shine(light_intensity, light_distance, light_direction, info.PHit);
			
				if (light_direction == glm::vec3(0.f, 0.f, -1.0f))
					shadow_ray->direction = -light_direction;
				
				shadow_ray->direction = -light_direction;
				shadow_ray->origin = info.PHit + info.NHit * HIT_BIAS;
				
#if defined(BBAccel)
				shadow_ray->precomputeValues();
#endif
				///use hit - light distance as maximum distance to trace, this will return actual distance.
				PHit_dist = light_distance;

				if (!is_lit) {
					break; 
}

				if (accel->intersect(shadow_ray, PHit_dist, shadow_info)!=nullptr) {
					is_lit = false;
					break;
					///TODO pridat zde nejaky testy jestli je object opaque a lomit svetlo? :) stinitko by mozna hodilo duhu
				}
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

					GetHitProperties(hit_triangle[0], hit_triangle[1], hit_triangle[2], info.u, info.v, texture.height, texture.width, N, tex_coords);

					uint32_t texelIndex = 3 * (glm::clamp((int)tex_coords.x, 0, texture.width - 1) + (texture.width)*glm::clamp((int)tex_coords.y, 0, texture.height - 1));

					texture_diffuse_color = glm::u8vec3(texture.data[0 + texelIndex],
						texture.data[1 + texelIndex],
						texture.data[2 + texelIndex]
					);
				}
				else
				{
					GetHitProperties(hit_triangle[0], hit_triangle[1], hit_triangle[2], info.u, info.v, N);
				}
				glm::vec3 ref_dir = light_direction;
#ifdef SMOOTH_SHADING
				d += (float)is_lit * hit_mesh->_albedo * light_intensity * glm::f32vec3(std::max(0.f, glm::dot(N, -light_direction)));
				Ray::calcReflectedDirection(N, ref_dir);
#else
				d += (float)is_lit * hit_mesh->_albedo * light_intensity * glm::f32vec3(std::max(0.f, glm::dot(primary_ray->hit_normal, -light_direction)));
				Ray::calcReflectedDirection(primary_ray->hit_normal, ref_dir);
#endif
				s += (float)is_lit * light_intensity * std::pow(std::max(0.f, glm::dot(ref_dir, -ray_dir)), hit_mesh->_material.shininess);
#else
				pixel_color = glm::clamp(glm::pow(pixel_color + hit_mesh->color*hit_color, glm::f32vec3(GAMMA)), 0.f, 1.f);
#endif
			}//end for each light in the scene

			if (hit_mesh->GetTextures().empty())
			{
				if (monochromatic && primary_ray->wavelength != 0) {
					glm::f32vec3 ray_color = Ray::wavelength2rgb(primary_ray->wavelength);
					pixel_color = glm::clamp((ray_color)*((/*1.0f*hit_mesh->_material.emissive_color +*/ d * hit_mesh->_material.diffuse_color + s * hit_mesh->_material.specluar_color) + hit_mesh->_material.ambient_color*AMBIENT_LIGHT), 0.f, 1.f);
				}
				else
				{
					pixel_color = glm::clamp((1.0f*hit_mesh->_material.emissive_color + d * hit_mesh->_material.diffuse_color + s * hit_mesh->_material.specluar_color + hit_mesh->_material.ambient_color*AMBIENT_LIGHT), 0.f, 1.f);
				}
			}
			else
			{
				if (monochromatic&& primary_ray->wavelength != 0) {
					pixel_color = glm::clamp(Ray::wavelength2rgb(primary_ray->wavelength)*((1.0f*hit_mesh->_material.emissive_color + d * U8vec2F32vec(texture_diffuse_color) + s * U8vec2F32vec(texture_diffuse_color) + U8vec2F32vec(texture_diffuse_color)*AMBIENT_LIGHT)), 0.f, 1.f);
				}
				else {
					pixel_color = glm::clamp((1.0f*hit_mesh->_material.emissive_color + d * U8vec2F32vec(texture_diffuse_color) + s * U8vec2F32vec(texture_diffuse_color) + U8vec2F32vec(texture_diffuse_color)*AMBIENT_LIGHT), 0.f, 1.f);
				}
			}
			delete(shadow_ray);
		}
		break;
		case RT_Mesh::DIFFUSE:
		default:

			shadow_ray = new Ray();
			is_lit = true;
			Ray::Hitinfo shadow_info;			
			//for each light cast shadow ray
			/// CALC LIGHTs
			for (auto &light : light_list) {
				//hit_color = glm::f32vec3(0);
				is_lit = true;
				OUT glm::vec3 light_intensity; OUT glm::vec3 light_direction; OUT float light_distance;
				light->shine(light_intensity, light_distance, light_direction, info.PHit);

				shadow_ray->direction = -light_direction;
				shadow_ray->origin = info.PHit + primary_ray->hit_normal /*NHit*/ * HIT_BIAS;
				shadow_ray->prev_D = primary_ray->prev_D;///COMMENT THIS OUT EVERYWHERE
#ifdef BVH_ACCEL
				shadow_ray->precomputeValues();
#elif defined(BBAccel)
				shadow_ray->precomputeValues();
#endif
				PHit_dist = light_distance;

					if (!is_lit) { break; }
					if (accel->intersect(shadow_ray, PHit_dist, shadow_info)!=nullptr) {
						is_lit = false;
						break;
					}
					if (monochromatic&& primary_ray->wavelength != 0) {
						hit_color += Ray::wavelength2rgb(primary_ray->wavelength)*((hit_mesh->_albedo)*light_intensity*glm::f32vec3(std::max(0.f, glm::dot(shadow_ray->hit_normal, -light_direction))));
					}
					else {
						hit_color += (hit_mesh->_albedo)*light_intensity*glm::f32vec3(std::max(0.f, glm::dot(shadow_ray->hit_normal, -light_direction)));
					}
				pixel_color = pixel_color + (float)is_lit*hit_mesh->_material.diffuse_color*hit_color;

			}//end for each light in the scene
			pixel_color = glm::clamp(pixel_color + hit_mesh->_material.ambient_color *AMBIENT_LIGHT,0.f,1.f);
			delete(shadow_ray);
			break;
		}
		
	}//end if primary ray hit mesh
	delete(primary_ray);		
	return pixel_color;
}//end for each mesh SHADOW RAYS


void render(uint16_t thread_id,
#ifdef MULTI_THREADING
	SDL_Rect rect,
#endif
	SDL_Surface* frame_buffer,
	std::chrono::time_point<std::chrono::steady_clock> &start,
	SDL_Event &event,
	Camera &camera,
	std::unique_ptr<AccelerationStructure> &accel,
	SDL_Texture * texture,
	SDL_Renderer * renderer,
	std::vector<int>kernel_wh
#ifdef MULTI_THREADING
	,std::vector<glm::u16vec2> min_whs
#endif
)
{
	glm::f32vec3 pixel_color = glm::f32vec3(0);
	//actual projection x and y in (-1,1)
	float view_x; float view_y; 
	//projection steps to fit inside WxH
	float height_step; float width_step;
	//SDL_Rect rect;
	///set rectangle coords
#ifdef MULTI_THREADING
	uint16_t min_w = min_whs[thread_id][0];	uint16_t min_h = min_whs[thread_id][1];
	uint16_t max_w = min_w + kernel_wh[0];	uint16_t max_h = min_h +kernel_wh[1];
	rect = {min_w,min_h,kernel_wh[0],kernel_wh[1]};
#else
	uint16_t min_w; uint16_t min_h;
	uint16_t max_w; uint16_t max_h;
	min_w = min_h = 0;
	max_w = WIDTH; max_h = HEIGHT;
#endif

#ifdef PROFILING
	start = std::chrono::high_resolution_clock::now();
	std::atomic_fetch_and(&primary_rays, 0);
	std::atomic_fetch_and(&triangle_intersection_count, 0);
	std::atomic_fetch_and(&triangle_test_count, 0);
#ifdef BBAccel
	std::atomic_fetch_and(&AccelerationStructure::box_test_count, 0);
#endif
#ifdef BVH_ACCEL
	std::atomic_fetch_and(&AccelerationStructure::num_ray_volume_tests, 0);
#endif
#endif
#ifdef SCREEN_SPACE_SUBSAMPLE
	char mod = 0;
#endif
	width_step = COORDS_FLOAT_WIDTH / (float)WIDTH;
	height_step = COORDS_FLOAT_HEIGHT / (float)HEIGHT;

	for (uint16_t y = min_h; y < max_h; ++y) {
		while (SDL_PollEvent(&event)) {
			MovePolling(event, camera);
		}
#ifdef SCREEN_SPACE_SUBSAMPLE
		if ((y - min_h) %SCREEN_SPACE_SUBSAMPLE) {
			for (int x = 0; x < kernel_wh[0]; ++x) {
				if (SCREEN_SPACE_SUBSAMPLE == 1) {
					setRGBAPixel(frame_buffer, x, y - min_h, getRGBAPixel(frame_buffer, x, y - min_h - 1));
				}
				else {
					if ((y - min_h)%SCREEN_SPACE_SUBSAMPLE <= (SCREEN_SPACE_SUBSAMPLE / 2))
					{
						if ((y - min_h) > SCREEN_SPACE_SUBSAMPLE) {
							setRGBAPixel(frame_buffer, x, y - min_h - (SCREEN_SPACE_SUBSAMPLE / 2 + 1), getRGBAPixel(frame_buffer, x, y - min_h - 1));
						}
						setRGBAPixel(frame_buffer, x , y - min_h, getRGBAPixel(frame_buffer, x, y - min_h - 1));
					}
				}
			}
			continue;
		}
		mod ^= 1;
#endif
		for (uint16_t x = min_w; x < max_w; ++x) {
#ifdef SCREEN_SPACE_SUBSAMPLE

			if ((x - min_w + mod) % SCREEN_SPACE_SUBSAMPLE) {
				if (SCREEN_SPACE_SUBSAMPLE == 1) {
					setRGBAPixel(frame_buffer, x - min_w, y - min_h, F32vec2U8vec(pixel_color));
				}
				else {
					if ((x - min_w )%SCREEN_SPACE_SUBSAMPLE <= (SCREEN_SPACE_SUBSAMPLE / 2))
					{
						if ((x - min_w) > SCREEN_SPACE_SUBSAMPLE) {
							setRGBAPixel(frame_buffer, x - min_w - (SCREEN_SPACE_SUBSAMPLE / 2 + 1), y - min_h, F32vec2U8vec(pixel_color));
						}
						setRGBAPixel(frame_buffer, x - min_w, y - min_h, F32vec2U8vec(pixel_color));
					}
				}
				continue;
			}

#endif
#ifdef MSAA	
			glm::f32vec3 cumulative_color = glm::f32vec3(0);
			for (uint8_t sample = 0; sample < msaa_sample_coords.size(); ++sample) {
				view_x = (-1.f + (width_step*((float)x + msaa_sample_coords[sample][0]))) * camera.scale * camera.aspect_ratio;
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
			cumulative_color += raytrace(accel,origin, direction);
			}
		pixel_color = cumulative_color / (float)msaa_sample_coords.size();
#else
			pixel_color = raytrace(accel, origin, direction);
#endif			
			setRGBAPixel(frame_buffer, x-min_w, y-min_h, F32vec2U8vec(pixel_color));

		}//end for each pixel (x)

#define FINAL_RENDER
#ifdef FINAL_RENDER
	}//end for each line y  - whole image processed
#endif
	
	 //SDL_LockSurface(frame_buffer);
	//
								//SDL_UnlockSurface(frame_buffer);
#ifdef MULTI_THREADING
	SDL_UpdateTexture(texture, NULL, frame_buffer->pixels, rect.w * sizeof(Uint32));
	std::lock_guard<std::mutex> block_threads_until_finish_this_job(barrier);
	SDL_RenderCopy(renderer, texture, NULL, &rect);
#else
	SDL_UpdateTexture(texture, NULL, frame_buffer->pixels, WIDTH * sizeof(Uint32));
	SDL_RenderCopy(renderer, texture, NULL, NULL);
#endif

	//SDL_RenderPresent(renderer);

#ifndef FINAL_RENDER
}
#endif
g_working--;
}
#ifdef MULTI_THREADING
inline bool divisible(unsigned int x, uint16_t factor) {
	return (x % factor == 0);
}

bool subimageDimensions(std::vector<int>& dim,uint16_t w, uint16_t h, uint16_t* num_o_parts) {

	if (divisible(w*h, num_o_parts[0])) {
		float floor_root = std::floorf(std::sqrtf(num_o_parts[0]));
		if (divisible(h, floor_root)) {
			dim[1] = (h) / floor_root;
			//dim[0] = (w*h) / (num_o_parts*dim[1]);
			dim[0] = w/(num_o_parts[0] / (h / dim[1]));
		}
		else {
			///TODO find closest
			return false;
		}
	}
	else {
		return false;
	}
	num_o_parts[0] = w / dim[0];
	num_o_parts[1] = h / dim[1];
	return true;
}
#endif
int main(int argc, char* argv[])
{/*
#ifdef _DEBUG
	_crtBreakAlloc = -1;
#endif*/
	SDL_Window* main_window;
	SDL_Event event;
	SDL_SetRelativeMouseMode(SDL_FALSE);

	///CAMERA
	Camera camera = Camera(glm::vec3(0.f,-0.5f,4.5f), glm::vec3(0.f,0.f,-1.f), 50.f, (float)WIDTH/ (float)HEIGHT);
	
	SDL_Init(SDL_INIT_VIDEO);
	main_window = SDL_CreateWindow("Raytracer",SDL_WINDOWPOS_UNDEFINED,SDL_WINDOWPOS_UNDEFINED,WIDTH,HEIGHT,SDL_WINDOW_OPENGL);
	if (main_window==NULL) {
		std::cerr << "SDL2 Main window creation failed.";
		return 1;
	}

	SDL_Renderer* renderer = SDL_CreateRenderer(main_window, -1, 0);

	
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

	mesh_list = std::move(LoadScene(modelPath));

#if defined(BVH_ACCEL)
std::unique_ptr<AccelerationStructure> accel(new BVH(mesh_list));
#elif defined(BBAccel)
	std::unique_ptr<AccelerationStructure> accel(new BBoxAcceleration(mesh_list));
#else
	std::unique_ptr<AccelerationStructure> accel(new AccelerationStructure(mesh_list));
#endif

	init_wave_info();
	//CreatePointLight(glm::vec3(-3.f, -0.8f, 0.f), 100.f, glm::f32vec3(U2F(255), U2F(255), U2F(255)));
	CreatePointLight(glm::vec3(0.f, -0.8f, -5.f), 400.f, glm::f32vec3(U2F(255), U2F(255), U2F(255)));
	//CreatePointLight(glm::vec3(-0.5f, -1.0f, 2.f), 100.f, glm::f32vec3(U2F(244), U2F(174), U2F(66)));
	
	CreateGlobalLight(glm::vec3(0.f, 0.f, -1.f), global_light_intensity, glm::f32vec3(U2F(255), U2F(255), U2F(255)));

	updateSkyColor();

	float view_x;
	float view_y;

#ifdef MULTI_THREADING
	g_thread_count = 2*std::thread::hardware_concurrency();
	std::cout << "Using " << g_thread_count << " threads.\n";

	std::vector<int> wh(2);
	uint16_t num_o_parts[2] = { g_thread_count ,0};
	if(!subimageDimensions(wh,WIDTH, HEIGHT, num_o_parts)){
		return 3;
	}
	std::vector<SDL_Surface*> frame_buffers;//= CreateRGBImage(WIDTH, HEIGHT);
	std::vector<SDL_Texture*> texture_buffers;
	std::vector<std::thread> threads;	
#endif

#ifdef PROFILING
	std::chrono::steady_clock::duration elapsed;
	std::chrono::steady_clock::time_point start;
	unsigned long long microseconds;
#endif

	SDL_Texture* texture;

#ifdef MULTI_THREADING
	SDL_Rect rect;
	std::vector<glm::u16vec2> min_whs;
	///fill min_whs with starting coords of rectangles in image for all threads
	for (uint8_t y=0; y < num_o_parts[1]; ++y) {
		for (uint8_t x=0; x < num_o_parts[0]; ++x) {
			SDL_Surface* frame_buffer = CreateRGBImage(wh[0], wh[1]);
			SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, frame_buffer);
			if (frame_buffer == NULL) {
				std::cout << "Failed create rgb surface";
				return 2;
			}
			frame_buffers.push_back(frame_buffer);
			texture_buffers.push_back(texture);
			min_whs.push_back(glm::vec2(x*wh[0],y*wh[1]));
		}
	}
#endif	
	while (!quit)
	{
		//SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
	//	SDL_RenderClear(renderer);
#ifdef MULTI_THREADING
		g_working = g_thread_count-1;
		for (uint16_t t = 0; t < num_o_parts[0]*num_o_parts[1]-1; ++t) {
			SDL_Surface* frame_buffer = frame_buffers[t];
			SDL_Texture* texture = texture_buffers[t];
			//render (t,OUT rect,std::ref(frame_buffer), std::ref(start), std::ref(event), std::ref(camera), std::ref(accel), std::ref(texture), std::ref(renderer), wh, std::ref(min_whs));
			threads.push_back(std::thread(render,t, rect, frame_buffer, std::ref(start), std::ref(event), std::ref(camera), std::ref(accel), texture, std::ref(renderer), wh, min_whs));

			//void render(uint16_t thread_id, SDL_Rect& rect, SDL_Surface* frame_buffer, std::chrono::time_point<std::chrono::steady_clock> &start, SDL_Event &event, Camera &camera, std::unique_ptr<AccelerationStructure> &accel, SDL_Texture * texture, SDL_Renderer * renderer, std::vector<int>kernel_wh, std::vector<glm::u16vec2> min_whs)

			//SDL_RenderCopy(renderer, texture, NULL, &rect);
		}

		SDL_Surface* frame_buffer = frame_buffers[num_o_parts[0]*num_o_parts[1]-1];
		SDL_Texture* texture = texture_buffers[num_o_parts[0] * num_o_parts[1]-1];

		render(g_thread_count - 1,rect, frame_buffer, start, event, camera, accel, texture, renderer, wh, min_whs);

		for (auto &thread:threads) {
			thread.join();
		}
#else
		SDL_Surface* frame_buffer = CreateRGBImage(WIDTH, HEIGHT);
		SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, frame_buffer);

		render(1, frame_buffer, start, event, camera, accel, texture, renderer, std::vector<int> {WIDTH,HEIGHT});
#endif
		SDL_RenderPresent(renderer);

		///cleanup
#ifdef MULTI_THREADING
		for (auto &thread:threads) {
			thread.~thread();	
		}
		threads.clear();
#endif
		

#ifdef PROFILING
		elapsed = std::chrono::high_resolution_clock::now() - start;
		microseconds = std::chrono::duration_cast<std::chrono::microseconds>(elapsed).count();
		std::cout << "Frametime: " << std::fixed<<std::setprecision(2)<< (float)microseconds/1000000.f<<"s";
		std::cout << "; " << std::fixed << std::setprecision(2) << 1000000.f/(float)microseconds << "	fps" << std::endl;
		std::cout << "Primary rays: " << std::fixed << std::setprecision(2) << primary_rays/1000.f<< "K" << std::endl;
#ifdef BBAccel
		std::cout << "BBox tests: " << std::fixed << std::setprecision(3) << AccelerationStructure::box_test_count / 1000000.f << "M" << std::endl;
#endif
//#if defined(BVH_ACCEL)
		std::cout << "Volumes tested: " << std::fixed << std::setprecision(3) << accel->getVolumeTestCount() / 1000.f << "K" << std::endl;
		std::cout << "BBoxes tested: " << std::fixed << std::setprecision(3) << accel->getBoxTestCount() / 1000000.f << "M" << std::endl;
//#endif
		std::cout << "Triangles intersected: " <<std::fixed<<std::setprecision(3) << triangle_intersection_count/ 1000.f << "K" << std::endl;
		std::cout << "Triangles tested: " << std::fixed <<std::setprecision(3) << triangle_test_count /1000000.f << "M \n" << std::endl;
		std::flush(std::cout);
#endif
#ifdef ONE_FRAME
		break;
#endif
	}
#ifdef MULTI_THREADING
	frame_buffers.clear();
	texture_buffers.clear();
#endif

	return 0;//app_exit(0, texture, renderer, frame_buffer, main_window);
}