#include <iostream>
#include <string.h>
#include <vector>
#include <SDL.h>
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <stdlib.h>
#include <string>

//Enable external gpu on a laptop supporting nvidia optimus
#include <Windows.h>
extern "C" {
	_declspec(dllexport) DWORD NvOptimusEnablement = 0x00000001;
}

#define NEAR_PLANE 0.1f
#define FAR_PLANE 100.0f

const float toRadians = glm::pi<float>() / 180.0f;

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
	SDL_Surface* rendered_image;
	//rendered_image = SDL_LoadBMP("");
//	rendered_image->pixels[x+640*y] = 

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
	}

	SDL_DestroyWindow(main_window);

	SDL_Quit();

	return 0;
}

