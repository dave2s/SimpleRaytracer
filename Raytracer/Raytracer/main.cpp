#include <iostream>
#include <string.h>
#include <vector>
#include <SDL.h>
#include <SDL_pixels.h>
#include <SDL_render.h>
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <stdlib.h>
#include <string>

#include "RT_Mesh.h"

//Enable external gpu on a laptop supporting nvidia optimus
#include <Windows.h>
extern "C" {
	_declspec(dllexport) DWORD NvOptimusEnablement = 0x00000001;
}

#define NEAR_PLANE 0.1f
#define FAR_PLANE 100.0f

const float toRadians = glm::pi<float>() / 180.0f;

void setRGBAPixel(SDL_Surface* rendered_image, Uint8 r, Uint8 g, Uint8 b, Uint8 a) {
	for (int x = 0; x < 640; ++x) {
		for (int y = 0; y < 480; ++y) {
			//rendered_image->pixels[x + 640 * y] = 
			unsigned char* pixels = (unsigned char*)rendered_image->pixels;
			pixels[4 * (y*rendered_image->w + x) + 0] = r;//blue
			pixels[4 * (y*rendered_image->w + x) + 1] = g;//green
			pixels[4 * (y*rendered_image->w + x) + 2] = b;//red
			pixels[4 * (y*rendered_image->w + x) + 3] = a;//alpha
		}
	}
}

int main(int argc, char* argv[])
{
	SDL_Window* main_window;
	SDL_Init(SDL_INIT_VIDEO);

	main_window = SDL_CreateWindow("Raytracer",
		SDL_WINDOWPOS_UNDEFINED,
		SDL_WINDOWPOS_UNDEFINED,
		640,
		480,
		SDL_WINDOW_OPENGL);

	if (main_window==NULL) {
		std::cerr << "SDL2 Main window creation failed.";
		return 1;
	}
	bool quit = false;
	SDL_Event event;

	SDL_Renderer* renderer = SDL_CreateRenderer(main_window, -1, 0);
	SDL_Surface* rendered_image = SDL_CreateRGBSurface(0,640,480,32,(Uint32)0xff000000, (Uint32)0x00ff0000, (Uint32)0x0000ff00, (Uint32)0x000000ff);
	if (rendered_image == NULL) {
		std::cout << "Failed create rgb surface";
		return 2;
	}
	
	setRGBAPixel(rendered_image, 255,255,0,255);

	SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, rendered_image);
	SDL_Rect dstrect = { 5, 5, 320, 240 };

	//SDL_Delay(30000);

	while (!quit)
	{
		SDL_WaitEvent(&event);

		switch (event.type)
		{
		case SDL_QUIT:
			quit = true;
			break;
		}

		//SDL_RenderCopy(renderer, texture, NULL, &dstrect);
		SDL_RenderCopy(renderer, texture, NULL, NULL);
		SDL_RenderPresent(renderer);
	}

	SDL_DestroyTexture(texture);
	SDL_FreeSurface(rendered_image);
	SDL_DestroyWindow(main_window);
	SDL_Quit();

	return 0;
}

