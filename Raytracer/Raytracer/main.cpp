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
#include <thread>
#include "SDL.h"
#include <SDL_pixels.h>
#include <SDL_render.h>
#include <SDL_ttf.h>
#include <algorithm>
#include <sstream>
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
#include "Defines.h"
#include <glm/gtc/random.hpp>
#include "BBoxAcceleration.h"
#include "BVH.h"
#include "AccelerationStructure.h"
#include "main.h"
#include <mutex>
#include <atomic>

//Enable external gpu on a laptop supporting nvidia optimus
/*#include <Windows.h>
extern "C" {
	_declspec(dllexport) DWORD NvOptimusEnablement = 0x00000001;
}*/


static std::atomic<unsigned long long> triangle_intersection_count = 0;
static std::atomic<unsigned long long> triangle_test_count = 0;
static std::atomic<unsigned long long> primary_rays = 0;
static std::atomic<unsigned char> global_light_on = true;
/*#ifdef BBAccel
static std::atomic<unsigned long long> box_test_count = 0;
#endif*/


//Borosilicate glass BK7
float B = 1.5046f;
float C = 0.00420f;

//float B = 2.37f;
//float C = 0.24f;

//Diamond
//float B = 2.385f;
//float C = 0.0117f;

float global_cauchy_B = B;
float global_cauchy_C = C;
//storing average ior
float average_ior;

///TODO
//#define GAMMA 1.7f
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
static std::atomic<unsigned long> total_triangle_count=0;
//std::vector<std::unique_ptr<const RT_Mesh>> refractive_mesh_list;
std::vector<RT_Light*> light_list;
std::vector<RT_Light*> light_list_off;

///Dispersion constants
std::vector<int> wavelengths;
int wavelengths_size;
static std::atomic<unsigned char> global_refraction = false;
static std::atomic<unsigned char> settings_changed = true;

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

int app_exit(int return_code,SDL_Texture* texture, SDL_Renderer* renderer, SDL_Surface* frame_buffer, SDL_Window* main_window)
{
	std::cout << "Ragequit();" << std::endl;
	SDL_DestroyTexture(texture);
	SDL_FreeSurface(frame_buffer);
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(main_window);
	SDL_Quit();
	
	mesh_list.erase(mesh_list.cbegin(), mesh_list.cend());
	//len = light_list.size();
	for (size_t i = light_list.size() - 1; i >= 0; --i) {
		delete(light_list.at(i));
	}
	///Free the vectors by redeclaring
	mesh_list.clear(); light_list.clear();

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

void MovePolling(SDL_Event &event,Camera &camera, std::unique_ptr<AccelerationStructure> &accel, uint32_t d_time) {
	//SDL_PollEvent(&event);
	//bool left_click;
	//float tick_ms=  1000 / float(FRAMES_PER_SECOND);
	float lag_compensation = 1.f;
	if (d_time < ONE_TICK_MS) {
		lag_compensation = 1.f;
	}
	lag_compensation = (lag_compensation > 5.f) ? 5.f : lag_compensation;

	if (event.type == SDL_KEYDOWN) {
		switch (event.key.keysym.sym) {
		case SDLK_w:
			camera.camera_position[2] -= lag_compensation*CONTROLS_MULTIPLIER*0.25f;
			break;
		case SDLK_s:
			camera.camera_position[2] += lag_compensation*CONTROLS_MULTIPLIER * 0.25f;
			break;
		case SDLK_a:
			camera.camera_position[0] -= lag_compensation*CONTROLS_MULTIPLIER * 0.25f;
			break;
		case SDLK_d:
			camera.camera_position[0] += lag_compensation*CONTROLS_MULTIPLIER * 0.25f;
			break;
		case SDLK_KP_PLUS:
			camera.fovy = (camera.fovy <=6 ) ? camera.fovy : camera.fovy - 5.f;
			camera.UpdateFov();
			break;
		case SDLK_KP_MINUS:
			camera.fovy = (camera.fovy >= 90) ? camera.fovy : camera.fovy + 5.f;
			camera.UpdateFov();
			break;
		///SETTINGS FOR DISPERSION
		case SDLK_h:
			barrier.lock();
			global_cauchy_B = (global_cauchy_B >=5.f)? global_cauchy_B : global_cauchy_B + 0.1f;
			barrier.unlock();
			std::atomic_fetch_xor(&settings_changed, 1);
			break;
		case SDLK_b:
			barrier.lock();
			global_cauchy_B = (global_cauchy_B <= 1.1f) ? global_cauchy_B : global_cauchy_B - 0.1f;
			barrier.unlock();
			std::atomic_fetch_xor(&settings_changed, 1);
			break;
		case SDLK_f:
			barrier.lock();
			global_cauchy_C = (global_cauchy_C >= .99f) ? global_cauchy_C : global_cauchy_C + 0.001f;
			barrier.unlock();
			std::atomic_fetch_xor(&settings_changed, 1);
			break;
		case SDLK_c:
			barrier.lock();
			global_cauchy_C = (global_cauchy_C <= -0.1f) ? global_cauchy_C : global_cauchy_C - 0.001f;
			barrier.unlock();
			std::atomic_fetch_xor(&settings_changed, 1);
			break;
		case SDLK_q:
			std::atomic_fetch_xor(&global_refraction, 1);
			std::atomic_fetch_xor(&settings_changed, 1);
			break;

		case SDLK_LEFT:
			if (!light_list.empty())
				((RT_PointLight*)light_list.at(0))->_position[0] -= lag_compensation*CONTROLS_MULTIPLIER * .1f;
			break;
		case SDLK_RIGHT:
			if (!light_list.empty())
				((RT_PointLight*)light_list.at(0))->_position[0] += lag_compensation*CONTROLS_MULTIPLIER * .1f;
			break;
		case SDLK_UP:
			if (!light_list.empty())
				((RT_PointLight*)light_list.at(0))->_position[1] += lag_compensation*CONTROLS_MULTIPLIER * .1f;
			break;
		case SDLK_DOWN:
			if (!light_list.empty())
				((RT_PointLight*)light_list.at(0))->_position[1] -= lag_compensation*CONTROLS_MULTIPLIER * .1f;
			break;
		case SDLK_KP_2:
			if (!light_list.empty())
				((RT_PointLight*)light_list.at(0))->_position[2] += lag_compensation*CONTROLS_MULTIPLIER * .1f;
			break;
		case SDLK_KP_8:
			if (!light_list.empty())
				((RT_PointLight*)light_list.at(0))->_position[2] -= lag_compensation*CONTROLS_MULTIPLIER * .1f;
			break;
		case SDLK_x:
			camera.camera_position[1] -= lag_compensation*CONTROLS_MULTIPLIER * 0.1f;
			break;
		case SDLK_r:
			camera.camera_position = { 0.f ,0.f,0.0f};
			break;
		case SDLK_SPACE:
			camera.camera_position[1] += lag_compensation*CONTROLS_MULTIPLIER * 0.1f;
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

float my_rand() {
	return glm::linearRand(0.f, 1.f);
}

void init_wave_info(){
	int step = int((wavelengths_intervals.back() - wavelengths_intervals.front()) / WAVE_SAMPLES);
	int wavelength_max = wavelengths_intervals.back();
	for (int wave = wavelengths_intervals.front()/*+int(step/2.f)*/; wave < wavelength_max; wave += step ) {
		wavelengths.push_back(wave);
	}
	wavelengths_size = wavelengths.size();
	
	/*average_ior = 0.f;
	uint16_t wavelen;
	for (int int_idx = 0; int_idx < wavelengths_intervals.size() - 1; ++int_idx) {
		 wavelen = wavelengths_intervals[int_idx] + 0.5*(wavelengths_intervals[int_idx + 1] - wavelengths_intervals[int_idx]);
			average_ior += Ray::iorFromWavelength(wavelen,global_cauchy_B,global_cauchy_C);
	}
	average_ior /= ((wavelengths_intervals.size()-1));*/
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
	float min_dist = inf;
	
	//__int64 mesh_iter_index;
	float prev_D;
	///For every mesh stored
	//mesh_iter_index = -1;
	prev_D = 0;

	primary_ray = new Ray();
	primary_ray->direction = ray_dir;
	primary_ray->origin = origin;
	primary_ray->isMonochrom = monochromatic;
	primary_ray->wavelength = wavelength;

#if defined (BBAccel)
	primary_ray->precomputeValues();
#endif	
	float PHit_dist = inf;
	Ray::Hitinfo info;
	const RT_Mesh* hit_mesh = accel->intersect(primary_ray, PHit_dist, info);
	if (hit_mesh != nullptr) {	
	///IF HIT DECIDE MATERIAL	
		//pixel_color = glm::f32vec3(0.f);
		int material_type;
		if (hit_mesh->_material_type == RT_Mesh::REFRACTION) {
			material_type = global_refraction ? RT_Mesh::REFRACTION : RT_Mesh::PHONG;
		}
		else {
			material_type = hit_mesh->_material_type;
		}
		glm::f32vec3 hit_color = glm::f32vec3(0);

		switch (material_type)
		{
		case RT_Mesh::MIRROR:
		{
			pixel_color = glm::f32vec3(0.f);
			glm::vec3 direction = primary_ray->direction;
			Ray::calcReflectedDirection(info.NHit, direction);
			min_dist = inf;
#define REFLECTION_BIAS
#ifdef REFLECTION_BIAS
			glm::vec3 PHit = info.PHit + info.NHit*HIT_BIAS;
			hit_color += 0.2f*hit_mesh->_material.diffuse_color + 0.8f *hit_mesh->_material.specular_color* raytrace(accel, info.PHit, direction, depth + 1,monochromatic,wavelength);
#else
			hit_color += 0.2f*hit_mesh->_material.diffuse_color + 0.8f *hit_mesh->_material.specular_color* raytrace(accel, info.PHit, direction, depth + 1,monochromatic,wavelength);
#endif
			pixel_color = glm::clamp(hit_color, 0.f, 1.f);
		}
		break;
		case RT_Mesh::REFRACTION:
		{
				hit_color = glm::f32vec3(0.f);
				glm::f32vec3 refract_color = glm::f32vec3(0);
				glm::f32vec3 reflect_color = glm::f32vec3(0);

				OUT float kr; min_dist = inf;
#ifdef SMOOTH_SHADING
				Vertex hit_triangle[3];
				std::memcpy(hit_triangle, hit_mesh->getTriangle(info.tri_idx), sizeof(Vertex) * 3);
				GetHitProperties(hit_triangle[0], hit_triangle[1], hit_triangle[2], info.u, info.v, primary_ray->hit_normal);
				//glm::vec3 bias = N_current * HIT_BIAS;
#endif
				glm::vec3 bias = primary_ray->hit_normal * HIT_BIAS;
				bool outside = glm::dot(primary_ray->direction, primary_ray->hit_normal) < 0.f;
				if (!monochromatic) {
					//SAMPLE WAVELENGTHS
					//uint16_t wavelen = 0;
					//for (int int_idx = 0; int_idx < wavelengths_intervals.size() - 1; ++int_idx) {
						//for (int sample = 0; sample < WAVE_SAMPLES; ++sample) {
					uint16_t num_refracted = 0;
					for(auto wavelen: wavelengths){
							//wavelen= wavelengths_intervals[int_idx] + my_rand()*(wavelengths_intervals[int_idx + 1] - wavelengths_intervals[int_idx]);
							//CALC FRESNEL
						float wavelen_ior = Ray::iorFromWavelength(wavelen, global_cauchy_B, global_cauchy_C);
							Ray::fresnel(wavelen_ior, kr, primary_ray->direction, primary_ray->hit_normal);
							// REFRACT IF NOT TOTAL INTERNAL REFLECTION
							
							if (kr < 1) {
								//lamu
								glm::vec3 refract_dir = glm::normalize(Ray::refract(/*hit_mesh->_material.ior*/wavelen_ior, primary_ray->direction, primary_ray->hit_normal));
								glm::vec3 refract_orig = outside ? info.PHit - bias : info.PHit + bias;
								refract_color += raytrace(accel, refract_orig, refract_dir, depth + 1, true, wavelen);
								num_refracted++;
							}
						//}
					}
					//refract_color /= (wavelengths_intervals.size() - 1)*WAVE_SAMPLES;
					refract_color /= num_refracted;
				//refract_color = glm::clamp(refract_color, 0.f, 1.f);
				}
				else {
					//CALC FRESNEL
					Ray::fresnel(Ray::iorFromWavelength(primary_ray->wavelength, global_cauchy_B, global_cauchy_C), kr, primary_ray->direction, primary_ray->hit_normal);
					// REFRACT IF NOT TOTAL INTERNAL REFLECTION
					if (kr < 1) {
						glm::vec3 refract_dir = glm::normalize(Ray::refract(/*hit_mesh->_material.ior*/Ray::iorFromWavelength(primary_ray->wavelength, global_cauchy_B, global_cauchy_C), primary_ray->direction, primary_ray->hit_normal));
						glm::vec3 refract_orig = outside ? info.PHit - bias : info.PHit + bias;
						refract_color += raytrace(accel, refract_orig, refract_dir, depth + 1, true, primary_ray->wavelength);
					}
				}

				glm::vec3 reflect_dir = primary_ray->direction;
				Ray::calcReflectedDirection(primary_ray->hit_normal, reflect_dir);
				glm::vec3 reflect_orig = outside ? info.PHit + bias : info.PHit - bias;
				reflect_color = raytrace(accel,reflect_orig, reflect_dir, depth + 1,monochromatic,wavelength);

				hit_color += hit_mesh->_material.specular_color*reflect_color * kr + hit_mesh->_material.refraction_color*refract_color * (1 - kr);
				pixel_color = glm::clamp(hit_color,0.f,1.f);
		}
		break;
		case RT_Mesh::PHONG:
		{
			pixel_color = glm::f32vec3(0.f);
			shadow_ray = new Ray();
			//is_lit = true;
			Ray::Hitinfo shadow_info;

			glm::f32vec3 d = glm::f32vec3(0); glm::f32vec3 s = glm::f32vec3(0);
			Vertex hit_triangle[3];
			std::memcpy(hit_triangle, hit_mesh->getTriangle(info.tri_idx), sizeof(Vertex) * 3);
			//for each light cast shadow ray
			/// CALC LIGHTs
			//shadow_ray->prev_D = primary_ray->prev_D;///COMMENT THIS OUT EVERYWHERE
			//const RT_Mesh* hit_shadow_mesh;
			///get textures
			glm::f32vec3 ambient_color = hit_mesh->_material.ambient_color;
			glm::f32vec3 diffuse_color = hit_mesh->_material.diffuse_color;
			glm::f32vec3 specular_color = hit_mesh->_material.specular_color;

			glm::vec2 tex_coords = glm::vec2(-1.f, -1.f);
			Texture texture; glm::vec3 N_current;
			if (!hit_mesh->GetTextures().empty())
			{
				uint32_t texelIndex;
				for (auto texItr = hit_mesh->GetTextures().begin(); texItr != hit_mesh->GetTextures().end(); ++texItr)
				{
					// TODO - should be strcmp
					if ((*texItr).type == "texture_diffuse")
					{
						texture = (*texItr);

						GetHitProperties(hit_triangle[0], hit_triangle[1], hit_triangle[2], info.u, info.v, texture.height, texture.width, N_current, tex_coords);

						texelIndex = texture.channels * (glm::clamp((int)tex_coords.x, 0, texture.width - 1) + (texture.width)*glm::clamp((int)tex_coords.y, 0, texture.height - 1));

						diffuse_color = U8vec2F32vec(glm::u8vec3(texture.data[0 + texelIndex],
							texture.data[1 + texelIndex],
							texture.data[2 + texelIndex]
						));
					}
					else if ((*texItr).type == "texture_ambient")
					{
						texture = (*texItr);

						GetHitProperties(hit_triangle[0], hit_triangle[1], hit_triangle[2], info.u, info.v, texture.height, texture.width, N_current, tex_coords);

						texelIndex = texture.channels * (glm::clamp((int)tex_coords.x, 0, texture.width - 1) + (texture.width)*glm::clamp((int)tex_coords.y, 0, texture.height - 1));

						ambient_color = U8vec2F32vec(glm::u8vec3(texture.data[0 + texelIndex],
							texture.data[1 + texelIndex],
							texture.data[2 + texelIndex]
						));
					}
					else if ((*texItr).type == "texture_specular")
					{
						texture = (*texItr);

						GetHitProperties(hit_triangle[0], hit_triangle[1], hit_triangle[2], info.u, info.v, texture.height, texture.width, N_current, tex_coords);

						texelIndex = texture.channels * (glm::clamp((int)tex_coords.x, 0, texture.width - 1) + (texture.width)*glm::clamp((int)tex_coords.y, 0, texture.height - 1));

						specular_color = U8vec2F32vec(glm::u8vec3(texture.data[0 + texelIndex],
							texture.data[1 + texelIndex],
							texture.data[2 + texelIndex]
						));
					}
					else if ((*texItr).type == "texture_bump")
					{
						texture = (*texItr);

						GetHitProperties(hit_triangle[0], hit_triangle[1], hit_triangle[2], info.u, info.v, texture.height, texture.width, N_current, tex_coords);

						texelIndex = texture.channels * (glm::clamp((int)tex_coords.x, 0, texture.width - 1) + (texture.width)*glm::clamp((int)tex_coords.y, 0, texture.height - 1));

						float height_displacement = U2F(texture.data[0 + texelIndex])-0.5f;
						info.PHit += height_displacement*info.NHit;						
					}
					else if ((*texItr).type == "texture_normal")
					{
						glm::f32vec3 t; glm::f32vec3 b;
						glm::f32vec3 texture_displacement;
						texture = (*texItr);
						GetHitProperties(hit_triangle[0], hit_triangle[1], hit_triangle[2], info.u, info.v, texture.height, texture.width, N_current, tex_coords,t,b);

						texelIndex = texture.channels * (glm::clamp((int)tex_coords.x, 0, texture.width - 1) + (texture.width)*glm::clamp((int)tex_coords.y, 0, texture.height - 1));

						/*glm::vec3 */t = glm::cross(N_current, glm::vec3(0.0f, 1.0f, 0.0f));
						/*glm::vec3 b;*/
						if (!glm::length(t)) {
							t = glm::cross(N_current, glm::vec3(0.0, 0.0, -1.0));
						}
						t = glm::normalize(t);
						b = glm::normalize((glm::cross(N_current, t)));

						//texelIndex = texture.channels * (glm::clamp((int)tex_coords.x, 0, texture.width - 1) + (texture.width)*glm::clamp((int)tex_coords.y, 0, texture.height - 1));
						texture_displacement = glm::f32vec3(texture.data[0 + texelIndex],texture.data[1 + texelIndex],texture.data[2 + texelIndex]);
						//map_n * 2 - 1
						texture_displacement = texture_displacement*2.f - glm::vec3(1.f);
						glm::mat3 tbn = glm::mat3(t, b, N_current);

						///Gram - Schmidt process we can re - orthogonalize
						/*glm::vec3 T = glm::normalize(glm::vec3(model * glm::vec4(aTangent, 0.0)));
						glm::vec3 N_current = glm::normalize(glm::vec3(model * glm::vec4(aNormal, 0.0)));
						// re-orthogonalize T with respect to N_current
						T = normalize(T - glm::dot(T, N_current) * N_current);
						// then retrieve perpendicular vector B with the cross product of T and N_current
						//glm::vec3 B = cross(N_current, T);

						//glm::mat3 TBN = glm::mat3(T, B, N_current);
*/
						N_current = glm::normalize(tbn * texture_displacement);
					/*	texture = (*texItr);
						if (texture.path.find(std::string("ddn.")) != std::string::npos) {

						}
						else {
							texelIndex = 3 * (glm::clamp((int)tex_coords.x, 0, texture.width - 1) + (texture.width)*glm::clamp((int)tex_coords.y, 0, texture.height - 1));
							texture_displacement
								= (texture.data[0 + texelIndex]);

							info.PHit += texture_displacement * N_current;
						}*/
					}
				}
			}
			else{
				GetHitProperties(hit_triangle[0], hit_triangle[1], hit_triangle[2], info.u, info.v, N_current);
			}
			OUT glm::vec3 light_intensity; OUT glm::vec3 light_direction; OUT float light_distance;
			for (auto &light : light_list) {
				
				light->shine(light_intensity, light_distance, light_direction, info.PHit);
				if (light_intensity.r < HIT_BIAS && light_intensity.g < HIT_BIAS && light_intensity.b < HIT_BIAS) {
					continue;
				}			
				shadow_ray->direction = -light_direction;
				///use hit - light distance as maximum distance to trace, this will return actual distance.
				PHit_dist = light_distance;
#ifndef SMOOTH_SHADING
				shadow_ray->origin = info.PHit + primary_ray->hit_normal * HIT_BIAS;
#else
				shadow_ray->origin = info.PHit + N_current * HIT_BIAS;
#endif
#if defined(BBAccel)
				//must have direction
				shadow_ray->precomputeValues();
#endif
				if (accel->intersect(shadow_ray, PHit_dist, shadow_info) != nullptr) {
					continue;
					///TODO pridat zde nejaky testy jestli je object opaque a lomit svetlo? :) stinitko by mozna hodilo duhu
				}

				glm::vec3 ref_dir = light_direction;
#ifdef SMOOTH_SHADING
				d +=   light_intensity * glm::f32vec3(std::max(0.f, glm::dot(N_current, -light_direction)));
				Ray::calcReflectedDirection(N_current, ref_dir);
#else
				d +=  light_intensity * glm::f32vec3(std::max(0.f, glm::dot(primary_ray->hit_normal, -light_direction)));
				Ray::calcReflectedDirection(primary_ray->hit_normal, ref_dir);
#endif
				s +=  light_intensity * std::pow(std::max(0.f, glm::dot(ref_dir, -ray_dir)), hit_mesh->_material.shininess);
			}//end for each light in the scene

			pixel_color = (1.0f*hit_mesh->_material.emissive_color + d /** hit_mesh->_albedo*/ * diffuse_color + s * specular_color +ambient_color*AMBIENT_LIGHT);

			if (monochromatic&& primary_ray->wavelength != 0) {
				pixel_color = glm::clamp(Ray::wavelength2rgb(primary_ray->wavelength)*pixel_color,0.f,1.f);
			}
			else {
				pixel_color = glm::clamp(pixel_color, 0.f, 1.f);
			}
			//}
			delete(shadow_ray);
		break;
		}


		case RT_Mesh::DIFFUSE:
		default:
			pixel_color = glm::f32vec3(0.f);
			shadow_ray = new Ray();
			is_lit = true;
			Ray::Hitinfo shadow_info;		
			Vertex hit_triangle[3];
			glm::f32vec3 hit_color = glm::f32vec3(0.f);
			std::memcpy(hit_triangle, hit_mesh->getTriangle(info.tri_idx), sizeof(Vertex) * 3);
			glm::f32vec3 N;
			//for each light cast shadow ray
			/// CALC LIGHTs
			for (auto &light : light_list) {
				//hit_color = glm::f32vec3(0);

				OUT glm::vec3 light_intensity; OUT glm::vec3 light_direction; OUT float light_distance;
				light->shine(light_intensity, light_distance, light_direction, info.PHit);

				shadow_ray->direction = -light_direction;
				shadow_ray->origin = info.PHit + primary_ray->hit_normal /*NHit*/ * HIT_BIAS;
				shadow_ray->prev_D = primary_ray->prev_D;///COMMENT THIS OUT EVERYWHERE
#ifdef BBAccel
				shadow_ray->precomputeValues();
#endif
				PHit_dist = light_distance;

				if (accel->intersect(shadow_ray, PHit_dist, shadow_info)!=nullptr) {
					continue;
				}

				GetHitProperties(hit_triangle[0], hit_triangle[1], hit_triangle[2], info.u, info.v, N);

				if (monochromatic&& primary_ray->wavelength != 0) {
					hit_color += Ray::wavelength2rgb(primary_ray->wavelength)*(hit_mesh->_material.diffuse_color*(hit_mesh->_albedo)*light_intensity*glm::f32vec3(std::max(0.f, glm::dot(N, -light_direction))));
				}
				else {
					hit_color +=hit_mesh->_material.diffuse_color*(hit_mesh->_albedo)*light_intensity*glm::f32vec3(std::max(0.f, glm::dot(N, -light_direction)));
				}
				//pixel_color = pixel_color + (float)is_lit*hit_mesh->_material.diffuse_color*hit_color;

			}//end for each light in the scene
			pixel_color = glm::clamp(hit_color + hit_mesh->_material.emissive_color + hit_mesh->_material.ambient_color *AMBIENT_LIGHT, 0.f,1.f);
			delete(shadow_ray);
			break;
		}
		
	}//end if primary ray hit mesh
	else {
		pixel_color = sky_color_actual;
	}
	delete(primary_ray);	
#ifdef GAMMA
	return glm::clamp(glm::f32vec3(glm::pow(pixel_color, glm::f32vec3(1.f/GAMMA))), 0.f, 1.f);
#else
	return pixel_color;
#endif

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

#ifdef SCREEN_SPACE_SUBSAMPLE
	char mod = 0;
#endif
	width_step = COORDS_FLOAT_WIDTH / (float)WIDTH;
	height_step = COORDS_FLOAT_HEIGHT / (float)HEIGHT;

	for (uint16_t y = min_h; y < max_h; ++y) {
		/*while (SDL_PollEvent(&event)) {
			MovePolling(event, camera,accel);
		}*/
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
			pixel_color = raytrace(accel, origin, direction,0,false,0);
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
#ifdef UPSCALE
	SDL_Rect r = {min_w*UPSCALE,min_h*UPSCALE,kernel_wh[0] * UPSCALE,kernel_wh[1] * UPSCALE };
	SDL_RenderCopy(renderer, texture, NULL, &r);
#else
	SDL_RenderCopy(renderer, texture, NULL, &rect);
#endif
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
void display_settings(SDL_Window* settings_window) {
	
		SDL_Renderer* renderer = SDL_CreateRenderer(settings_window, -1, 0);;
		SDL_SetRenderDrawColor(renderer, 200, 200, 200, 0);
		SDL_RenderClear(renderer);
		SDL_RenderPresent(renderer);
		if (TTF_Init() < 0) {
			std::cerr << "TTF_Init() failed" << std::endl;
		}
		using namespace std;
		if (TTF_WasInit()) {
			///TODO USE VECTORS FOR CLEAN CODE's SAKE
			SDL_Surface* surfaceMessage_C = nullptr;
			SDL_Surface* surfaceMessage_B = nullptr;
			SDL_Surface* surfaceMessage_Cauchy = nullptr;
			SDL_Surface* surfaceMessage_ior400 = nullptr;
			SDL_Surface* surfaceMessage_ior700 = nullptr;
			SDL_Surface* surfaceMessage_refractive = nullptr;
			TTF_Font* Sans = TTF_OpenFont("OpenSans-Regular.ttf", 100); //this opens a font style and sets a size
			SDL_Rect rect_b = { 0,0,200,25 }; //create a rect
			SDL_Rect rect_c = { 0,35,200,25 }; //create a rect
			SDL_Rect rect_cauchy = { 220,18,200,25 }; //create a rect
			SDL_Rect rect_ior400 = { 220,50,200,25 }; //create a rect
			SDL_Rect rect_ior700 = { 220,82,200,25 }; //create a rect
			SDL_Rect rect_refractive = { 0,125,100,25 }; //create a rect
			SDL_Color Violet = { 131, 0, 181 };  // this is the color in rgb format, maxing out all would give you the color white, and it will be your text's color
			SDL_Color TextColor = { 100, 100, 200 };  // this is the color in rgb format, maxing out all would give you the color white, and it will be your text's color
			SDL_Color Red = { 255, 0, 0 };  // this is the color in rgb format, maxing out all would give you the color white, and it will be your text's color
			ostringstream _B;
			ostringstream _C;
			ostringstream _ior400;
			ostringstream _ior700;

			while (!quit) {

				if (settings_changed) {
					SDL_SetRenderDrawColor(renderer, 0, 0, 0, 1);
					SDL_RenderClear(renderer);

					_B << "Cauchy parametr B: " << std::fixed << std::setprecision(3) << global_cauchy_B;
					_C << "Cauchy parametr C: " << std::fixed << std::setprecision(4) << global_cauchy_C;
					_ior400 << "IOR(400nm): " << std::fixed << std::setprecision(3) << Ray::iorFromWavelength(400,global_cauchy_B,global_cauchy_C);
					_ior700 << "IOR(700nm): " << std::fixed << std::setprecision(3) << Ray::iorFromWavelength(700, global_cauchy_B, global_cauchy_C);
					surfaceMessage_B = TTF_RenderText_Solid(Sans, _B.str().c_str(), TextColor);
					surfaceMessage_C = TTF_RenderText_Solid(Sans, _C.str().c_str(), TextColor);
					surfaceMessage_ior400 = TTF_RenderText_Solid(Sans, _ior400.str().c_str(), Violet);
					surfaceMessage_ior700 = TTF_RenderText_Solid(Sans, _ior700.str().c_str(), Red);
					surfaceMessage_Cauchy = TTF_RenderText_Solid(Sans, string("n(lambda) = B + (Cx5) / lambda^2").c_str(), TextColor);
					surfaceMessage_refractive = TTF_RenderText_Solid(Sans, string("Refraction: "+(global_refraction?string("ON"):string("OFF"))).c_str(), TextColor);

					SDL_Texture* Message_Cauchy = nullptr;SDL_Texture* Message_B = nullptr;SDL_Texture* Message_C = nullptr;SDL_Texture* Message_ior400 = nullptr;SDL_Texture* Message_ior700 = nullptr;SDL_Texture* Message_refractive = nullptr;
					Message_B = SDL_CreateTextureFromSurface(renderer, surfaceMessage_B);
					Message_C = SDL_CreateTextureFromSurface(renderer, surfaceMessage_C);
					Message_Cauchy = SDL_CreateTextureFromSurface(renderer, surfaceMessage_Cauchy);
					Message_ior400 = SDL_CreateTextureFromSurface(renderer, surfaceMessage_ior400);
					Message_ior700 = SDL_CreateTextureFromSurface(renderer, surfaceMessage_ior700);
					Message_refractive = SDL_CreateTextureFromSurface(renderer, surfaceMessage_refractive);

					SDL_RenderCopy(renderer, Message_B, NULL, &rect_b);
					SDL_RenderCopy(renderer, Message_C, NULL, &rect_c);
					SDL_RenderCopy(renderer, Message_Cauchy, NULL, &rect_cauchy);
					SDL_RenderCopy(renderer, Message_ior400, NULL, &rect_ior400);
					SDL_RenderCopy(renderer, Message_ior700, NULL, &rect_ior700);
					SDL_RenderCopy(renderer, Message_refractive, NULL, &rect_refractive);
					SDL_RenderPresent(renderer);

					SDL_FreeSurface(surfaceMessage_B); SDL_FreeSurface(surfaceMessage_C); SDL_FreeSurface(surfaceMessage_Cauchy); SDL_FreeSurface(surfaceMessage_ior400); SDL_FreeSurface(surfaceMessage_ior700); SDL_FreeSurface(surfaceMessage_refractive);
					SDL_DestroyTexture(Message_Cauchy); SDL_DestroyTexture(Message_C);	SDL_DestroyTexture(Message_B); SDL_DestroyTexture(Message_B); SDL_DestroyTexture(Message_ior400); SDL_DestroyTexture(Message_ior700); SDL_DestroyTexture(Message_refractive);

					_B.str(string());
					_C.str(string());
					_ior400.str(string());
					_ior700.str(string());
					//Don't forget too free your surface and texture
					std::atomic_fetch_and(&settings_changed,0);
				}
			}
			TTF_CloseFont(Sans);
		}
		TTF_Quit();
}
int main(int argc, char* argv[])
{/*
#ifdef _DEBUG
	_crtBreakAlloc = -1;
#endif*/
	SDL_Window* main_window; SDL_Window* settings_window;
	SDL_Event event; std::thread settings_thread;
	SDL_SetRelativeMouseMode(SDL_FALSE);

	///CAMERA
	Camera camera = Camera(glm::vec3(0.f,0.f,6.f), glm::vec3(0.f,0.f,-1.f),90.f, (float)WIDTH/ (float)HEIGHT);
	
	SDL_Init(SDL_INIT_VIDEO);
#ifdef UPSCALE
	main_window = SDL_CreateWindow("Raytracer",SDL_WINDOWPOS_UNDEFINED,SDL_WINDOWPOS_UNDEFINED,WIDTH*UPSCALE,HEIGHT*UPSCALE,NULL);
#else
	main_window = SDL_CreateWindow("Raytracer",SDL_WINDOWPOS_UNDEFINED,SDL_WINDOWPOS_UNDEFINED,WIDTH,HEIGHT,SDL_WINDOW_OPENGL);
#endif
	if (main_window==NULL) {
		std::cerr << "SDL2 Main window creation failed.";
		return 1;
	}
	int main_window_h; int main_window_w;
	SDL_GetWindowSize(main_window,&main_window_w,&main_window_h);
	int main_window_x; int main_window_y;
	SDL_GetWindowPosition(main_window,&main_window_x,&main_window_y);
	settings_window = SDL_CreateWindow("RT_Settings",main_window_x+main_window_w , main_window_y, SETTINGS_WIDTH, SETTINGS_HEIGHT, NULL);
	if (settings_window == NULL) {
		std::cerr << "SDL2 Settings window creation failed.";
		return 1;
	}
	///Display settings
	settings_thread = std::thread(display_settings,std::ref(settings_window));

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
	for (const auto& mesh : mesh_list) {
		std::atomic_fetch_add(&total_triangle_count, mesh.get()->getTriangleCount());
	}
#if defined(BVH_ACCEL)
	std::unique_ptr<AccelerationStructure> accel;
	if (total_triangle_count > 50)
	{
		std::cout << "Total triangles in scene: "<< total_triangle_count <<" >50 . Using octree based BVH.\n";
		 accel = std::unique_ptr<AccelerationStructure>(new BVH(mesh_list));
	}
	else{
		std::cout << "Total triangles in scene: " << total_triangle_count << " <= 50. Using mesh bounding boxes only.\n";
		accel = std::unique_ptr<AccelerationStructure>(new BBoxAcceleration(mesh_list));
	}
#elif defined(BBAccel)
	std::unique_ptr<AccelerationStructure> accel(new BBoxAcceleration(mesh_list));
#else
	std::unique_ptr<AccelerationStructure> accel(new AccelerationStructure(mesh_list));
#endif

	init_wave_info();
	CreatePointLight(glm::vec3(-2.f, 1.0f, -2.f), 100.f, glm::f32vec3(U2F(66), U2F(174), U2F(244)));
	//CreatePointLight(glm::vec3(2.f, 1.0f, 2.f), 400.f, glm::f32vec3(U2F(244), U2F(174), U2F(66)));
	//CreatePointLight(glm::vec3(20.f, 50.0f, 20.f), 100000.f, glm::f32vec3(U2F(244), U2F(174), U2F(66)));
	//CreatePointLight(glm::vec3(-20.f, 50.0f, 20.f), 100000.f, glm::f32vec3(U2F(66), U2F(174), U2F(244)));
	
	//CreateGlobalLight(glm::vec3(0.f, .0f, -1.0f), global_light_intensity, glm::f32vec3(U2F(255), U2F(255), U2F(255)));
	CreateGlobalLight(glm::vec3(0.f, -1.f, -0.2f), global_light_intensity*1, glm::f32vec3(U2F(255), U2F(255), U2F(255)));
//	CreateGlobalLight(glm::vec3(-0.1f, -.2f,0.2f), global_light_intensity*1, glm::f32vec3(U2F(255), U2F(255), U2F(255)));

	updateSkyColor();

	float view_x;
	float view_y;

#ifdef MULTI_THREADING
	g_thread_count = 1*std::thread::hardware_concurrency();
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

	std::chrono::steady_clock::duration elapsed;
	std::chrono::steady_clock::time_point start;
	unsigned long long microseconds = 0;

	SDL_Texture* texture;

#ifdef MULTI_THREADING
	SDL_Rect rect;
	std::vector<glm::u16vec2> min_whs;
	///fill min_whs with starting coords of rectangles in image for all threads
	for (uint8_t y=0; y < num_o_parts[1]; ++y) {
		for (uint8_t x=0; x < num_o_parts[0]; ++x) {
			//SDL_Surface* frame_buffer = CreateRGBImage(wh[0], wh[1]);
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
	uint32_t t_last = 0;
	uint32_t lag = 0.f;
	uint32_t t_elapsed;
	uint32_t t_current;
	while (!quit)
	{
		t_current = SDL_GetTicks();
		t_elapsed = t_current - t_last;
		t_last = t_current;
		lag += t_elapsed;

		SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
		SDL_RenderClear(renderer);
		g_working = g_thread_count-1;
	
		//if (t_current >= (t_last + ONE_TICK_MS)) {
	//		d_time_event = t_current - t_last;
		while (lag >= ONE_TICK_MS){
			while (SDL_PollEvent(&event)) {
				MovePolling(event, camera, accel,lag/ONE_TICK_MS);
			}
		lag -= ONE_TICK_MS;
			//t_last = t_current;
		}
#ifdef MULTI_THREADING
		///multithreaded render loop
		for (uint16_t t = 0; t < num_o_parts[0]*num_o_parts[1]-1; ++t) {
			SDL_Surface* frame_buffer = frame_buffers[t];
			SDL_Texture* texture = texture_buffers[t];
			threads.push_back(std::thread(render,t, rect, frame_buffer, std::ref(start), std::ref(event), std::ref(camera), std::ref(accel), texture, std::ref(renderer), wh, min_whs));
		}

		SDL_Surface* frame_buffer = frame_buffers[num_o_parts[0]*num_o_parts[1]-1];
		SDL_Texture* texture = texture_buffers[num_o_parts[0] * num_o_parts[1]-1];
		//main loop renders one block too
		render(num_o_parts[0]*num_o_parts[1] - 1,rect, frame_buffer, start, event, camera, accel, texture, renderer, wh, min_whs);
#else
		SDL_Surface* frame_buffer = CreateRGBImage(WIDTH, HEIGHT);
		SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, frame_buffer);

		render(1, frame_buffer, start, event, camera, accel, texture, renderer, std::vector<int> {WIDTH,HEIGHT});
#endif
		//d_time_render = 
		while (SDL_GetTicks() < t_last + ONE_TICK_MS) {
			true;//std::this_thread::sleep_for(std::chrono::milliseconds(int(ONE_TICK_MS-(SDL_GetTicks()-t_last))));
		}
		///cleanup
#ifdef MULTI_THREADING
		for (auto &thread : threads) {
			thread.join();
		}
		SDL_RenderPresent(renderer);
		for (auto &thread:threads) {
			thread.~thread();	
		}
		threads.clear();

#else
		SDL_RenderPresent(renderer);
#endif
		

#ifdef PROFILING
		elapsed = std::chrono::high_resolution_clock::now() - start;
		microseconds = std::chrono::duration_cast<std::chrono::microseconds>(elapsed).count();
		std::cout << "Frametime: " << std::fixed<<std::setprecision(2)<< (float)microseconds/1000000.f<<"s";
		std::cout << "; " << std::fixed << std::setprecision(2) << 1000000.f/(float)microseconds << "	fps" << std::endl;
		std::cout << "Primary rays: " << std::fixed << std::setprecision(2) << primary_rays/1000000.f<< "M" << std::endl;
/*#ifdef BBAccel
		std::cout << "BBox tested: " << std::fixed << std::setprecision(3) << AccelerationStructure::box_test_count / 1000000.f << "M" << std::endl;
#endif*/
//#if defined(BVH_ACCEL)
		std::cout << "Volumes tested: " << std::fixed << std::setprecision(3) << accel->getVolumeTestCount() / 1000000.f << "M" << std::endl;
		std::cout << "BBoxes tested: " << std::fixed << std::setprecision(3) << accel->getBoxTestCount() / 1000000.f << "M" << std::endl;
//#endif
		std::cout << "Triangles intersected: " <<std::fixed<<std::setprecision(3) << accel->getTrianglesIntersected() / 1000000.f << "M" << std::endl;
		std::cout << "Triangles tested: " << std::fixed <<std::setprecision(3) << RT_Mesh::getTriangleTests() /1000000.f << "M \n" << std::endl;
		std::flush(std::cout);

		start = std::chrono::high_resolution_clock::now();

		std::atomic_fetch_and(&primary_rays, 0);
		std::atomic_fetch_and(&RT_Mesh::triangle_tests, 0);
		std::atomic_fetch_and(&AccelerationStructure::triangles_intersected, 0);
	#ifdef BBAccel
			std::atomic_fetch_and(&AccelerationStructure::box_test_count, 0);
	#endif
	#ifdef BVH_ACCEL
			std::atomic_fetch_and(&AccelerationStructure::num_ray_volume_tests, 0);
	#endif
#endif

#ifdef ONE_FRAME
		break;
#endif
	}
#ifdef MULTI_THREADING
	frame_buffers.clear();
	texture_buffers.clear();
#endif

	if(settings_thread.joinable())
		settings_thread.join();

	//_CrtDumpMemoryLeaks();

	return 0;//app_exit(0, texture, renderer, frame_buffer, main_window);
}