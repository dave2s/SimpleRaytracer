#include <iostream>
#include "Model.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

// Prevzato z https://learnopengl.com/Model-Loading/Model


std::vector<Texture> LoadTextures(aiMaterial *mtl, aiTextureType type, std::string typeName, std::string &dir)
{
	std::vector<Texture> textures;
	for (uint32_t i = 0; i < mtl->GetTextureCount(type); ++i)
	{
		aiString path;
		Texture tex;
		int width, height, channels;

		mtl->GetTexture(type, i, &path);
		stbi_set_flip_vertically_on_load(true);
		tex.data = stbi_load((dir + "/" + path.C_Str()).c_str(), &width, &height, &channels, 3);
		if (tex.data == nullptr)
		{
			continue;
		}

		tex.height = height;
		tex.width = width;
		tex.channels = channels;
		tex.type = typeName;
		tex.path = path.C_Str();
		textures.push_back(tex);
		
	}
	return textures;
}

std::unique_ptr<const RT_Mesh> ProcessTreeMesh(const aiScene* scene, aiMesh* mesh, std::string& dir) {
	std::vector< Vertex >vertices;
	std::vector<unsigned int> indices;
	std::vector<Texture> textures;
	//glm::f32vec3 color = glm::f32vec3(1.f,1.f,1.f);
	aiMaterial *mtl;
	Material my_material;

	//RT_Mesh::MATERIAL_TYPE type = RT_Mesh::DIFFUSE;
	RT_Mesh::MATERIAL_TYPE type = RT_Mesh::PHONG;

	//vertices = new RT_Mesh::Vertex[mesh->mNumVertices]();
	//vertices = new RT_Mesh::Vertex[mesh->mNumVertices]();
	glm::vec3 tmp;
	for (unsigned i = 0; i < mesh->mNumVertices; ++i) {
		Vertex v;
		tmp.x = mesh->mVertices[i].x;
		tmp.y = mesh->mVertices[i].y;
		tmp.z = mesh->mVertices[i].z;
		v.position = tmp;

		tmp.x = mesh->mNormals[i].x;
		tmp.y = mesh->mNormals[i].y;
		tmp.z = mesh->mNormals[i].z;
		v.normal = tmp;

		if (mesh->mTextureCoords[0]) {
			v._tex_coords = glm::vec2(mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y);
		}
		else {
			v._tex_coords = glm::vec2(0.f, 0.f);
		}
		vertices.push_back(v);
	}
	//indices = new unsigned int[mesh->mNumFaces];
	//indices = new unsigned int[mesh->mNumFaces]();
	for (unsigned int i = 0; i < mesh->mNumFaces; ++i) {
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
		aiColor4D emissive;
		aiColor4D global_refraction;
		float shininess = 1.f;
		float ior = 1.f;
		int shading_model = -1; 

		aiGetMaterialColor(mtl, AI_MATKEY_COLOR_DIFFUSE, &diffuse);
		aiGetMaterialColor(mtl, AI_MATKEY_COLOR_AMBIENT, &ambient);
		aiGetMaterialColor(mtl, AI_MATKEY_COLOR_SPECULAR, &specular);
		aiGetMaterialColor(mtl, AI_MATKEY_COLOR_EMISSIVE, &emissive);
		aiGetMaterialFloat(mtl, AI_MATKEY_SHININESS, &shininess);
		aiGetMaterialFloat(mtl, AI_MATKEY_REFRACTI, &ior);
		aiGetMaterialInteger(mtl, AI_MATKEY_SHADING_MODEL, &shading_model);
		aiGetMaterialColor(mtl, AI_MATKEY_COLOR_TRANSPARENT, &global_refraction);

		///TODO alpha
		my_material.refraction_color = glm::f32vec3(-1.f);

		my_material.ambient_color = (glm::f32vec4(ambient.r, ambient.g, ambient.b, ambient.a) == glm::f32vec4(0)) ? glm::f32vec3(AMBIENT_LIGHT) : glm::f32vec4(ambient.r, ambient.g, ambient.b, ambient.a);
		my_material.diffuse_color = glm::f32vec4(diffuse.r, diffuse.g, diffuse.b, diffuse.a);
		my_material.specular_color = glm::f32vec4(specular.r, specular.g, specular.b, specular.a);
		my_material.emissive_color = glm::clamp(glm::f32vec3(emissive.r, emissive.g, emissive.b),0.f,1.f);
		my_material.ior = ior;
		my_material.shininess = shininess;
		my_material.refraction_color = glm::f32vec4(global_refraction.r, global_refraction.g, global_refraction.b, global_refraction.a);
		
		/*if (shading_model != -1) {
			switch (shading_model) {
			case 0:
			case 1:
			case 2:
			case 3:
				type = RT_Mesh::PHONG;
				if (shininess > 999) {
					type = RT_Mesh::MIRROR;
				}
				break;
			case 4:
			case 5:
			case 6:
			case 7:
			case 8:
			case 9:
			case 10:
				type = RT_Mesh::REFRACTION;
				if (shininess > 999) {
					type = RT_Mesh::MIRROR;
				}
				break;
			}
		}*/
		if (shading_model == 2 || shading_model == 3){
			type = ( ior != 1.f && (my_material.refraction_color != glm::f32vec3(-1.f)) ) ? RT_Mesh::REFRACTION : RT_Mesh::PHONG;
			if (my_material.specular_color == glm::f32vec3(0))
				//type = RT_Mesh::DIFFUSE;
			if (shininess > 999) {
				type = RT_Mesh::MIRROR;
			}
		}
		else {
			/*if (shininess == 1.f) {
				type = RT_Mesh::DIFFUSE;
			}*/
			/*else {*/
				type = RT_Mesh::PHONG;
				if (shininess > 999) {
					type = RT_Mesh::MIRROR;
				}
			/*}*/
		}

		std::vector<Texture> diff_Map = LoadTextures(mtl, aiTextureType_DIFFUSE, "texture_diffuse",dir);
		textures.insert(textures.end(), diff_Map.begin(), diff_Map.end());
		std::vector<Texture> spec_map = LoadTextures(mtl, aiTextureType_SPECULAR, "texture_specular",dir);
		textures.insert(textures.end(), spec_map.begin(), spec_map.end());
		std::vector<Texture> displ_map = LoadTextures(mtl, aiTextureType_DISPLACEMENT, "texture_displ", dir);
		textures.insert(textures.end(), displ_map.begin(), displ_map.end());
	}

	//std::unique_ptr<const RT_Mesh> my_mesh = new std::unique_ptr(RT_Mesh(vertices, indices, false, my_material, 0.18f, type,textures));
	std::unique_ptr<const RT_Mesh> my_mesh(new RT_Mesh(vertices, indices, false, my_material, 0.18f, type, textures));
	return my_mesh;
}

void ProcessSceneTree(const aiScene* scene, std::vector<std::unique_ptr<const RT_Mesh>> &meshes, aiNode* node, std::string& dir)
{
	for (uint32_t i = 0; i < node->mNumMeshes; ++i)
	{
		aiMesh *mesh = scene->mMeshes[node->mMeshes[i]];
		meshes.emplace_back(ProcessTreeMesh(scene, mesh, dir));
	}

	for (uint32_t i = 0; i < node->mNumChildren; ++i)
	{
		ProcessSceneTree(scene, meshes, node->mChildren[i], dir);
	}
}

std::vector<std::unique_ptr<const RT_Mesh>> LoadScene(std::string& modelPath)
{
	std::vector<std::unique_ptr<const RT_Mesh>> meshes;
	Assimp::Importer importer;
	const aiScene* scene = nullptr;
	scene = importer.ReadFile(modelPath, aiProcess_FixInfacingNormals | aiProcess_Triangulate | aiProcess_GenSmoothNormals);
	if (!scene || !scene->mRootNode || scene->mFlags & (AI_SCENE_FLAGS_INCOMPLETE | AI_SCENE_FLAGS_VALIDATION_WARNING))
	{
		std::cerr << "Model importer failed. flags: " << scene->mFlags << std::endl;
	}
	std::string dir = modelPath.substr(0, modelPath.find_last_of('/'));
	aiNode* node = scene->mRootNode;
	ProcessSceneTree(scene, meshes, node, dir);
	return meshes;
}

