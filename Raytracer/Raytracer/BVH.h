#pragma once
/*
https://www.highperformancegraphics.org/wp-content/uploads/2015/Papers-Session1/apresentacao.pdf
Bounding Volume Hierarchy Optimization
through Agglomerative Treelet Restructuring

Fast Parallel Construction of High-Quality Bounding Volume Hierarchies
Tero Karras Timo Aila NVIDIA
https://research.nvidia.com/publication/fast-parallel-construction-high-quality-bounding-volume-hierarchies

*/

#include <cstdint>
#include "GLM/glm.hpp"
class BVH
{

public:

	BVH();
	~BVH();

	//store further
	struct SABounds {
		SABounds() {
			for (uint8_t i = 0; i < plane_count; ++i) {
				//d[i][0] = inf;
				//d[i][1] = -inf;
			}
		}
	};

private:
	static const uint8_t plane_count = 7;

	const glm::vec3 setPlanes[plane_count];
	
};

