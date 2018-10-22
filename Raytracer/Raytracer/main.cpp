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

//Enable external gpu on a laptop supporting nvidia optimus
#include <Windows.h>
extern "C" {
	_declspec(dllexport) DWORD NvOptimusEnablement = 0x00000001;
}
#define WIDTH 320.f
#define HEIGHT 240.f

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
		0,1,2,
		0,2,3,
		//zadni stena
		5,4,7,
		5,7,6,
		//bocni prava stena
		1,5,6,
		1,6,2,
		//bocni leva stena
		4,0,3,
		4,3,7
	};

	RT_Mesh* mesh = new RT_Mesh();		//indices vertices length pointer arithmetic 
	mesh->CreateMesh(vertices, indices, *(&vertices+1)-vertices, *(&indices + 1) - indices, false);
	mesh_list.push_back(mesh);

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
	CreatePointLight(glm::vec3(0.f, 100.f, 1.5f), 0.3f,glm::vec3(255,255,255));

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

	Ray *ray;
	float min_dist = inf;
	float PHit_dist;
	glm::vec3 PHit;

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
				int mesh_index_triangle[2] = {-1,-1};

				for (auto mesh = mesh_list.begin(); mesh != mesh_list.end(); ++mesh) {
					triangle_count = (*mesh)->getTriangleCount();//std::cout << std::to_string(triangle_count) << "\n";
					for (i = 0; i < triangle_count; ++i) {///For every triangle of the mesh
						ray = new Ray();
						(Ray::calcRayPerspectiveDirection(ray,
							view_x,
							view_y,
							1.0,
							CAM_NEAR_PLANE, camera));
						if (RT_Mesh::rayHitTriangle((*mesh)->getTriangle(i),
							/*ray.calcRayOrigin*/ray,
							camera,
							(*mesh)->isSingleSided(), PHit_dist,PHit, min_dist)
							) {
							if (PHit_dist < min_dist) {
								mesh_index_triangle[0] = mesh - mesh_list.begin();
								mesh_index_triangle[1] = i;
								min_dist = PHit_dist;
							}
						}
					}
				}
				if (!(mesh_index_triangle[0] == -1 || mesh_index_triangle[1] == -1)) {
					//setRGBAPixel(frame_buffer, x, y, 0, 255, 0, 255);
					Ray *shadowRay;
					bool is_lit = false;
					//for each light cast shadow ray
					/*for (auto light = light_list.begin(); light != light_list.end(); ++light){

						shadowRay->direction = (*light)->position - PHit;
					}*/
					is_lit = true;
					if (is_lit) {
						setRGBAPixel(frame_buffer, x, y, glm::u8vec4(0, 255, 0, 255));
					}
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

