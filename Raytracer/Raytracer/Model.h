#pragma once
#include <string>
#include <vector>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include "RT_Mesh.h"


std::vector<std::unique_ptr<const RT_Mesh>> LoadScene(std::string& modelPath);




