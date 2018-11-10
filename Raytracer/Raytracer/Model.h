#pragma once
#include <string>
#include <vector>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

class ModelLoader
{
public:
	ModelLoader();

	
	struct Texture {
		unsigned int id;
		std::string path;
		std::string type;
	};
	std::vector<Texture> textures;

	void load(std::string path);
	std::vector<Texture> loadTextures();

	~ModelLoader();
};

