#include <iostream>
#include <string.h>
#include <vector>
#include "SDL.h"
#include <SDL_pixels.h>
#include <SDL_render.h>
//#define GLM_FORCE_CUDA
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <stdlib.h>
#include <string>
#include "Camera.h"
#include "RT_Mesh.h"
#include "Ray.h"

//Enable external gpu on a laptop supporting nvidia optimus
#include <Windows.h>
extern "C" {
	_declspec(dllexport) DWORD NvOptimusEnablement = 0x00000001;
}
#define WIDTH 320
#define HEIGHT 240
#define COORDS_FLOAT_WIDTH 2.0f
#define COORDS_FLOAT_HEIGHT 2.0f

const float toRadians = glm::pi<float>() / 180.0f;

std::vector<RT_Mesh*> mesh_list;

glm::vec3 camera_position = glm::vec3(0.f,0.f,0.f);
glm::vec3 camera_direction = glm::vec3(0.f, 0.f, -1.f);

SDL_Surface* CreateRGBImage(int width, int height) {
	return SDL_CreateRGBSurface(0, width, height, 32, (Uint32)0xff000000, (Uint32)0x00ff0000, (Uint32)0x0000ff00, (Uint32)0x000000ff);
}

void setRGBAPixel(SDL_Surface* rendered_image,int x, int y, Uint8 r, Uint8 g, Uint8 b, Uint8 a) {
	//rendered_image->pixels[x + 640 * y] = 
	unsigned char* pixels = (unsigned char*)rendered_image->pixels;
	pixels[4 * (y*rendered_image->w + x) + 0] = r;//blue
	pixels[4 * (y*rendered_image->w + x) + 1] = g;//green
	pixels[4 * (y*rendered_image->w + x) + 2] = b;//red
	pixels[4 * (y*rendered_image->w + x) + 3] = a;//alpha
}

void CreateTriangle() {
	float vertices[] = {
		//predni stena
		-0.5f, 0.5f, 0.1f,
		0.5f, 0.5f, 0.1f,
		0.5f, -0.5f, 0.1f,
		-0.5f, -0.5f, 0.1f,
		//zadni stena
		-0.5f, 0.5f, 0.2f,
		0.5f, 0.5f, 0.2f,
		0.5f, -0.5f, 0.2f,
		-0.5f, -0.5f, 0.2f		
	};

	unsigned int indices[] = {
		//predni stena
		0,1,2,
		0,2,3,
		//zadni stena
		5,4,6,
		6,4,7,
		//bocni prava stena
		1,4,2,
		4,6,2
	};

	RT_Mesh* mesh = new RT_Mesh();		//indices vertices length pointer arithmetic 
	mesh->CreateMesh(vertices, indices, *(&vertices+1)-vertices, *(&indices + 1) - indices, true);
	mesh_list.push_back(mesh);

}

int main(int argc, char* argv[])
{
	SDL_Window* main_window;
	SDL_Event event;
	bool quit = false;
	Camera camera = Camera(glm::vec3(0.f,0.f,0.f));
	camera.CalcCamView(glm::vec3(0.f,0.f,0.15f));

	SDL_Init(SDL_INIT_VIDEO);
	main_window = SDL_CreateWindow("Raytracer",SDL_WINDOWPOS_UNDEFINED,SDL_WINDOWPOS_UNDEFINED,WIDTH+5,HEIGHT+5,SDL_WINDOW_OPENGL);
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

	//create triangle
	CreateTriangle();
	//for each pixel of the frame buffer set it's color
	//SDL_Rect dstrect = { 5, 5, 320, 240 };
	Ray ray;
	int triangle_count;
	int i;
	//SDL_Delay(30000);
	SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, frame_buffer);
	while (!quit)
	{
		SDL_WaitEvent(&event);

		switch (event.type)
		{
		case SDL_QUIT:
			quit = true;
			break;
		}
		float width_step = COORDS_FLOAT_WIDTH / (float)WIDTH;
		float height_step = COORDS_FLOAT_HEIGHT / (float)HEIGHT;
		for (int y = 0; y < HEIGHT; ++y) {
			for (int x = 0; x < WIDTH; ++x) {

				//Clear the texture black
				setRGBAPixel(frame_buffer, x, y, 0, 0, 0, 255);
				
				triangle_count = mesh_list[0]->getTriangleCount();
				//std::cout << std::to_string(triangle_count) << "\n";
				for ( i = 0; i < triangle_count; ++i) {
					//Jestli hadam spravne, tak pro orthogonalni projekci bude ray direction vzdy 0 0 1 a pozice bude posunuta tak, aby 
					//kdyz origin camery bude 0 0 0 ve stredu obrazu ale, tak origin paprsku bude posunuty podle x a u
					if (RT_Mesh::rayHitTriangle((mesh_list[0]->getTriangle(i)),
						/*ray.calcRayOrigin*/(glm::vec3(1.f, 0.f, 0.f)), (ray.calcRayPerspectiveDirection(glm::vec3(1.f, 0.f, 0.f),
							-1.f + (width_step*(float)x),
							-1.f + (height_step*(float)y),
							WIDTH,
							HEIGHT,
							CAM_NEAR_PLANE,camera)),
						camera,
						mesh_list[0]->isSingleSided())
						) {
						std::cout << "kreslim pro x: " << std::to_string(-1.f + (width_step*(float)x)) << " a y: " << std::to_string(-1.f + (height_step*(float)y)) << " \n";
						std::cout << "x: " << std::to_string(x) << "y: " << std::to_string(y) << "\n";
						//continue;
						setRGBAPixel(frame_buffer, x, y, 0, 255, 0, 255);
					}
				}
			}
			SDL_UpdateTexture(texture, NULL, frame_buffer->pixels, WIDTH * sizeof(Uint32));//SDL_CreateTextureFromSurface(renderer, frame_buffer);
			SDL_RenderClear(renderer);
			SDL_RenderCopy(renderer, texture, NULL, NULL);
			SDL_RenderPresent(renderer);
		}
		/*SDL_UpdateTexture(texture, NULL, frame_buffer->pixels, WIDTH * sizeof(Uint32));//SDL_CreateTextureFromSurface(renderer, frame_buffer);
		SDL_RenderClear(renderer);
		SDL_RenderCopy(renderer, texture, NULL, NULL);
		SDL_RenderPresent(renderer);*/
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

