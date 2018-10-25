#include <iostream>
#include <iomanip>
#include <string>
#include <vector>
#include <chrono>
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

#define SUBSAMPLE 1
#define PROFILING

//Enable external gpu on a laptop supporting nvidia optimus
/*#include <Windows.h>
extern "C" {
	_declspec(dllexport) DWORD NvOptimusEnablement = 0x00000001;
}*/
#define WIDTH 320.f
#define HEIGHT 240.f

#define AMBIENT_LIGHT_STRENGHT 0.3f

bool quit = false;

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

	float vertices[] = {
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

	unsigned int indices[] = {
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
		3,2,6,

		//spodni trojuhelnik
		1,0,4
	};

	glm::u8vec4 color = glm::u8vec4(1,255,1,255);

	RT_Mesh* box = new RT_Mesh();		//indices vertices length pointer arithmetic 
	box->CreateMesh(vertices, indices, *(&vertices+1)-vertices, *(&indices + 1) - indices, false,color);
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
		SDL_Delay(2);
		float y = LO0y + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / (HI0y - LO0x)));
		SDL_Delay(3);
		float z = LO0z + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / (HI0z - LO0x)));
		return glm::vec3(x,y,z);
}

void MovePolling(SDL_Event &event,Camera &camera) {
	SDL_PollEvent(&event);
	bool left_click;

	if (event.type == SDL_KEYDOWN) {
		switch (event.key.keysym.sym) {
		case SDLK_w:
			camera.camera_position[2] -= 0.01;
			break;
		case SDLK_s:
			camera.camera_position[2] += 0.01;
			break;
		case SDLK_a:
			camera.camera_position[0] -= 0.01;
			break;
		case SDLK_d:
			camera.camera_position[0] += 0.01;
			break;
		case SDLK_c:
			camera.camera_position[1] -= 0.01;
			break;
		case SDLK_SPACE:
			camera.camera_position[1] += 0.01;
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
	Camera camera = Camera(glm::vec3(0.0f,0.0f,1.5f), glm::vec3(0.f,0.f,-1.f),45.f, (float)WIDTH/ (float)HEIGHT);

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
	float LO0x = -2;
	float LO0y = -2;
	float LO0z = -2;
	float HI0x = 2;
	float HI0y = 2;
	float HI0z = 2;

	glm::vec3 random_light0_position = calcRandPos(LO0x, LO0y, LO0z, HI0x, HI0y, HI0z);
	SDL_Delay(300);
	glm::vec3 random_light1_position = calcRandPos(LO0x, LO0y, LO0z, HI0x, HI0y, HI0z);
	CreatePointLight(random_light0_position, 1.66f, glm::vec3(1, 255, 127));
	CreatePointLight(random_light1_position, 1.66f, glm::vec3(1, 255, 127));
	//CreatePointLight(glm::vec3(0.2f, -1.5f, 1.75f), 1.66f,glm::vec3(1,255,127));
	//CreatePointLight(glm::vec3(0.2f, 0.6f, 1.75f), 1.66f, glm::vec3(255,1,127));

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

	#ifdef SUBSAMPLE
	char mod = 0;
	#endif
	while (!quit)
	{
		SDL_RenderClear(renderer);

		SDL_PollEvent(&event);
		switch (event.type)
		{
		case SDL_QUIT:
			quit = true;
			break;
		}	

#ifdef PROFILING
		auto start = std::chrono::high_resolution_clock::now();
#endif
		for (int y = 0; y < HEIGHT; ++y) {
			//SDL_PollEvent(&event);
			MovePolling(event, camera);
#ifdef SUBSAMPLE
			//mod ^= 1;
			mod++; if (mod % SUBSAMPLE) {continue; }
			mod = 0;
#endif

			for (int x = 0; x < WIDTH; ++x) {
				//MovePolling(event, camera);
				//std::cout << std::to_string(mod+1) << std::endl;
#ifdef SUBSAMPLE
				mod++; if ((mod % (SUBSAMPLE+1))!=0)continue;
				mod = 0;
#endif

				setRGBAPixel(frame_buffer, x, y, glm::u8vec4(0, 0, 0, 255));

				view_x = (-1.f + (width_step*(float)x)) * camera.scale;
				view_y = (1.f - (height_step*(float)y)) * camera.scale * 1 / camera.aspect_ratio;

				min_dist = inf;
				///For every mesh stored
				__int64 mesh_iter_index = -1;
				int mesh_triangle_index = -1;

				for (auto mesh = mesh_list.begin(); mesh != mesh_list.end(); ++mesh) 
				{
					triangle_count = (*mesh)->getTriangleCount();//std::cout << std::to_string(triangle_count) << "\n";
					for (i = 0; i < triangle_count; ++i) {///For every triangle of the mesh
						primary_ray = new Ray();
						
						(Ray::calcRayPerspectiveDirection(primary_ray,
							view_x,
							view_y,
							1.0,
							CAM_NEAR_PLANE, camera));
						//std::cout << "x:" << x << " y: " << y << std::endl;
						if (x == 161 && y == 120) {
							std::cout << "primary ray dir:" << glm::to_string(primary_ray->direction)<<"origin"<<glm::to_string(primary_ray->origin)<< std::endl;
						}
						if (RT_Mesh::primaryRayHitTriangle((*mesh)->getTriangle(i),
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
				///IF HIT CALC LIGHTs
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
						//shadow_ray->direction = glm::normalize((*light)->position - closest_PHit);
						shadow_ray->direction = Ray::calcRayDirection(closest_PHit,(*light)->position);
						shadow_ray->origin = closest_PHit;

						//we won't intersect object further than this distance of light and the last hit
						min_dist = RT_Mesh::getDistanceFromOrigin(shadow_ray->direction, (*light)->position);
						//for each object
						for (auto mesh = mesh_list.begin(); mesh != mesh_list.end(); ++mesh) {
							//std::cout << "prochazim meshe" << std::endl;
							if (!is_lit)/*std::cout << "opoustim smycku meshu - nasel jsem stin" << std::endl;*/break;
							triangle_count = (*mesh)->getTriangleCount();//std::cout << std::to_string(triangle_count) << "\n";
							for (i = 0; i < triangle_count; ++i) {///For every triangle of the mesh
								//std::cout << "prochazim trojuhelniky" << std::endl;
								if (RT_Mesh::shadowRayHitTriangle((*mesh)->getTriangle(i),
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
			SDL_LockSurface(frame_buffer);			
			SDL_UpdateTexture(texture, NULL, frame_buffer->pixels, WIDTH * sizeof(Uint32));//
			SDL_UnlockSurface(frame_buffer);
			SDL_RenderCopy(renderer, texture, NULL, NULL);
			SDL_RenderPresent(renderer);
		}
#ifdef PROFILING
		auto elapsed = std::chrono::high_resolution_clock::now() - start;
		long long microseconds = std::chrono::duration_cast<std::chrono::microseconds>(elapsed).count();
		std::cout << "Frametime: " << std::fixed<<std::setprecision(2)<< (float)microseconds/1000000.f<<"us";
		std::cout << "; " << std::fixed << std::setprecision(2) << 1000000.f/(float)microseconds << "	fps" << std::endl;
		std::flush(std::cout);
#endif
		//std::cout << "\nKONEC KRESLENI\n";
		//std::cout << "\nPress enter to render again.\n";
		//std::cout << "Press ESC to exit.\n";
		//light_list[0]->position = calcRandPos(LO0x, LO0y, LO0z, HI0x, HI0y, HI0z);
		//light_list[1]->position = calcRandPos(LO0x, LO0y, LO0z, HI0x, HI0y, HI0z);
		/*bool render = 0;
		//Event polling for continuation of render or exit (RET/ESC)
		while (!render) {
			while (SDL_PollEvent(&event))   //Poll our SDL key event for any keystrokes.
			{
				if (event.type == SDL_QUIT)
					app_exit(0,texture, renderer, frame_buffer, main_window);
				if (event.type == SDL_KEYUP) {
					switch (event.key.keysym.sym) {
					case SDLK_ESCAPE:
						return app_exit(0,texture, renderer, frame_buffer, main_window);
					case SDLK_RETURN:
						//light_list[1]->position = calcRandPos(LO0x, LO0y, LO0z, HI0x, HI0y, HI0z);
						//SDL_Delay(300);						
						light_list[0]->position= calcRandPos(LO0x, LO0y, LO0z, HI0x, HI0y, HI0z);
						std::cout << "Rendering..." << std::endl;
						render = 1;
						break;
					default:
						continue;
					}
				}
			}
		}*/
	}
	app_exit(0, texture, renderer, frame_buffer, main_window);
	return 0;
}

