#pragma once
#include <string>
#include <vector>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include "RT_Mesh.h"

class Model
{
public:
	Model();

	/*struct Texture {
		unsigned int id;
		std::string path;
		std::string type;
	};*/

	static std::vector<RT_Mesh::Texture> textures;

	static void loadScene(std::string model_path, std::vector<RT_Mesh*>& meshes);
	//void processSceneTree(const aiScene* scene, std::vector<RT_Mesh*> meshes);


	~Model();
private:

	//const aiScene* scene;
	static void processSceneTree(const aiScene* scene, std::vector<RT_Mesh*>& meshes, aiNode* node);
	static RT_Mesh* processTreeMesh(const aiScene* scene, aiMesh* mesh);
//	std::vector<RT_Mesh::Texture> loadTextures();

	//std::vector<RT_Mesh::Texture> loadMaterialTextures(aiMaterial * mat, aiTextureType type, std::string typeName);
};




