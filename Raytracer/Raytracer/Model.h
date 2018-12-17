#pragma once
#include <string>
#include <vector>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include "RT_Mesh.h"
#define AMBIENT_LIGHT 0.075f


void LoadScene(std::string& modelPath, std::vector<RT_Mesh*>& meshes);




