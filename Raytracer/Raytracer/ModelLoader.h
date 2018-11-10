#pragma once
#include <string>
#include <vector>

class ModelLoader
{
public:
	ModelLoader();

	
	struct Texture {
		unsigned int id;
		std::string path;
		
	};
	std::vector<Texture> textures;

	~ModelLoader();
};

