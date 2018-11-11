#include <iostream>
#include "Model.h"



Model::Model()
{
}

void Model::loadScene(std::string model_path, std::vector<RT_Mesh*>& meshes) {
	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile(model_path, aiProcess_Triangulate | aiProcess_GenNormals /*| aiProcess_FlipUVs*/);
	if (!scene || !scene->mRootNode || scene->mFlags & (AI_SCENE_FLAGS_INCOMPLETE | AI_SCENE_FLAGS_VALIDATION_WARNING)) {
		std::cout << "Model importer failed. flags: " << scene->mFlags << std::endl;
	}

	aiNode* node = scene->mRootNode;
	processSceneTree(scene, meshes, node);
}
/*
void Model::processSceneTree(const aiScene* scene, std::vector<RT_Mesh*> meshes){
	for (int i = 0; i < node->mNumMeshes; ++i)
	{
		aiMesh *mesh = scene->mMeshes[node->mMeshes[i]];
		meshes.push_back(processTreeMesh(scene, mesh));
	}
	// then do the same for each of its children
	for (int i = 0; i < node->mNumChildren; ++i)
	{
		processSceneTree(scene, meshes, node->mChildren[i]);
	}
}*/

void Model::processSceneTree(const aiScene* scene, std::vector<RT_Mesh*> &meshes, aiNode* node){
	for (int i = 0; i < node->mNumMeshes; ++i)
	{
		aiMesh *mesh = scene->mMeshes[node->mMeshes[i]];
		meshes.push_back(processTreeMesh(scene, mesh));
	}

	for (int i = 0; i < node->mNumChildren; ++i)
	{
		processSceneTree(scene, meshes, node->mChildren[i]);
	}
}

RT_Mesh* Model::processTreeMesh(const aiScene* scene, aiMesh* mesh) {
	RT_Mesh::Vertex* vertices;
	unsigned int* indices;
	glm::f32vec3 color;
	RT_Mesh::MATERIAL_TYPE type;
	
	vertices = new RT_Mesh::Vertex[mesh->mNumVertices];
	glm::vec3 tmp;
	for (int i = 0; i < mesh->mNumVertices; ++i) {
		tmp.x = mesh->mVertices[i].x;
		tmp.y = mesh->mVertices[i].y;
		tmp.z = mesh->mVertices[i].z;
		vertices[i].position = tmp;

		tmp.x = mesh->mNormals[i].x;
		tmp.y = mesh->mNormals[i].y;
		tmp.z = mesh->mNormals[i].z;
		vertices[i].normal = tmp;

		if (mesh->mTextureCoords[0]) {
			vertices[i].tex_coords = glm::vec2(mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y);
		}
		else {
			vertices[i].tex_coords = glm::vec2(0.f, 0.f);
		}
	}
	indices = new unsigned int[mesh->mNumFaces];
	for (int i = 0; i < mesh->mNumFaces; ++i){
		aiFace face = mesh->mFaces[i];
		for (int j = 0; j < face.mNumIndices; ++j) {
			indices[i]=face.mIndices[j];
		}
	}

	/*if (mesh->mMaterialIndex >= 0)
	{
		aiMaterial *material = scene->mMaterials[mesh->mMaterialIndex];
		std::vector<RT_Mesh::Texture> diffuseMaps = loadMaterialTextures(material,
			aiTextureType_DIFFUSE, "texture_diffuse");
		textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());
		vector<Texture> specularMaps = loadMaterialTextures(material,
			aiTextureType_SPECULAR, "texture_specular");
		textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());
	}*/

	return new RT_Mesh(vertices, indices, *(&vertices + 1) - vertices, *(&indices + 1) - indices, false, color, 0.2f, type);
}

/*std::vector<RT_Mesh::Texture> loadMaterialTextures(aiMaterial *mat, aiTextureType type, std::string typeName)
{
	std::vector<RT_Mesh::Texture> textures;
	for (unsigned int i = 0; i < mat->GetTextureCount(type); i++)
	{
		aiString str;
		mat->GetTexture(type, i, &str);
		RT_Mesh::Texture texture;
		texture.id = TextureFromFile(str.C_Str(), directory);
		texture.type = typeName;
		texture.path = str;
		textures.push_back(texture);
	}
	return textures;
}*/

Model::~Model()
{
}
