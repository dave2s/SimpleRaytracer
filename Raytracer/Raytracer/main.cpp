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

#include "stb_image.h"
//image write

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

	SDL_Delay(30000);
	SDL_DestroyWindow(main_window);

	SDL_Quit();

	return 0;
}

