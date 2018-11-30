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
	for (unsigned int i = 0; i < node->mNumMeshes; ++i)
	{
		aiMesh *mesh = scene->mMeshes[node->mMeshes[i]];
		meshes.push_back(processTreeMesh(scene, mesh));
	}

	for (unsigned int i = 0; i < node->mNumChildren; ++i)
	{
		processSceneTree(scene, meshes, node->mChildren[i]);
	}
}

RT_Mesh* Model::processTreeMesh(const aiScene* scene, aiMesh* mesh) {
	std::vector< RT_Mesh::Vertex >vertices;
	std::vector<unsigned int> indices;
	//glm::f32vec3 color = glm::f32vec3(1.f,1.f,1.f);
	aiMaterial *mtl;
	RT_Mesh::Material my_material;

	//RT_Mesh::MATERIAL_TYPE type = RT_Mesh::DIFFUSE;
	RT_Mesh::MATERIAL_TYPE type = RT_Mesh::PHONG;
	
	//vertices = new RT_Mesh::Vertex[mesh->mNumVertices]();
	//vertices = new RT_Mesh::Vertex[mesh->mNumVertices]();
	glm::vec3 tmp;
	for (unsigned i = 0; i < mesh->mNumVertices; ++i) {
		RT_Mesh::Vertex v;
		tmp.x = mesh->mVertices[i].x;
		tmp.y = mesh->mVertices[i].y;
		tmp.z = mesh->mVertices[i].z;
		v.position = tmp;

		tmp.x = mesh->mNormals[i].x;
		tmp.y = mesh->mNormals[i].y;
		tmp.z = mesh->mNormals[i].z;
		v.normal= tmp;

		if (mesh->mTextureCoords[0]) {
			v.tex_coords = glm::vec2(mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y);
		}
		else {
			v.tex_coords = glm::vec2(0.f, 0.f);
		}
		vertices.push_back(v);
	}
	//indices = new unsigned int[mesh->mNumFaces];
	//indices = new unsigned int[mesh->mNumFaces]();
	for (unsigned int i = 0; i < mesh->mNumFaces; ++i){
		aiFace face = mesh->mFaces[i];
		for (unsigned int j = 0; j < face.mNumIndices; ++j) {
			indices.push_back(face.mIndices[j]);
		}
	}
	
	if (mesh->mMaterialIndex >= 0)
	{
		mtl = scene->mMaterials[mesh->mMaterialIndex];
		aiColor4D diffuse;
		aiColor4D ambient;
		aiColor4D specular;
		float shininess;
		aiGetMaterialColor(mtl, AI_MATKEY_COLOR_DIFFUSE, &diffuse);
		aiGetMaterialColor(mtl, AI_MATKEY_COLOR_AMBIENT, &ambient);
		aiGetMaterialColor(mtl, AI_MATKEY_COLOR_SPECULAR, &specular);
		aiGetMaterialFloat(mtl, AI_MATKEY_SHININESS, &shininess);
		
			///TODO alpha
		my_material.ambient_color = (glm::f32vec4(ambient.r, ambient.g, ambient.b, ambient.a)==glm::f32vec4(0))?glm::f32vec3(AMBIENT_LIGHT): glm::f32vec4(ambient.r, ambient.g, ambient.b, ambient.a);
		my_material.diffuse_color = glm::f32vec4(diffuse.r, diffuse.g, diffuse.b, diffuse.a);
		my_material.specluar_color = glm::f32vec4(specular.r, specular.g, specular.b, specular.a);
		my_material.shininess = shininess;

		if (shininess > 999)
			type = RT_Mesh::MIRROR;
		
		//color = glm::f32vec3(diffuse.r, diffuse.g, diffuse.b);

		//std::vector<RT_Mesh::Texture> diff_Map = loadTextures(mtl, aiTextureType_DIFFUSE, "texture_diffuse");
		//textures.insert(textures.end(), diff_Map.begin(), diff_Map.end());
		//std::vector<RT_Mesh::Texture> spec_map = loadTextures(mtl, aiTextureType_SPECULAR, "texture_specular");
		//textures.insert(textures.end(), spec_map.begin(), spec_map.end());
	}

	RT_Mesh* my_mesh = new RT_Mesh(vertices, indices, vertices.size(),indices.size(), true, my_material, 0.18f, type);
	return my_mesh;
}
/*
std::vector<RT_Mesh::Texture> loadTextures(aiMaterial *mat, aiTextureType type, std::string typeName)
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
