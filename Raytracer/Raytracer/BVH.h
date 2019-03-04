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
#include "Ray.h"
#include "AccelerationStructure.h"
#include "Defines.h"
#include <queue>
#include <atomic>

//static const float inf = std::numeric_limits<float>::max();
/*
*class taken from https://www.scratchapixel.com/lessons/advanced-rendering/introduction-acceleration-structure/bounding-volume-hierarchy-BVH-part2
*Authors of www.sratchapixel.com are unknown
*/
static std::atomic<unsigned long long> num_ray_volume_tests = 0;

class BVH : AccelerationStructure
{
	static const uint8_t plane_count = 7;
	//const glm::vec3 setPlanes[plane_count];

	static const glm::f32vec3 planeSetNormals[plane_count];
	struct Extents
	{
		Extents()
		{
			for (uint8_t i = 0; i < plane_count; ++i)
				d[i][0] = inf, d[i][1] = -inf;
		}
		void extendBy(const Extents &extents)
		{
			for (uint8_t i = 0; i < plane_count; ++i) {
				if (extents.d[i][0] < d[i][0]) d[i][0] = extents.d[i][0];
				if (extents.d[i][1] > d[i][1]) d[i][1] = extents.d[i][1];
			}
		}
		bool intersect(
			const float *precomputedNumerator, const float *precomputeDenominator,
			float &tNear, float &tFar, uint8_t &planeIndex);
		float d[plane_count][2]; // d values for each plane-set normals 
		const RT_Mesh* mesh; // pointer contained by the volume (used by octree) 
	};
	Extents *extents;
	struct OctreeNode
	{
		OctreeNode *child[8];
		std::vector<const Extents *> data;
		Extents extents;
		bool isLeaf;
		uint8_t depth; // just for debugging 
		OctreeNode() : isLeaf(true) { memset(child, 0x0, sizeof(OctreeNode *) * 8); }
		~OctreeNode() { for (uint8_t i = 0; i < 8; ++i) if (child[i] != NULL) delete child[i]; }
	};
	struct Octree
	{
		Octree(const Extents &extents) : root(NULL)
		{
			float xdiff = extents.d[0][1] - extents.d[0][0];
			float ydiff = extents.d[1][1] - extents.d[1][0];
			float zdiff = extents.d[2][1] - extents.d[2][0];
			float dim = std::max(xdiff, std::max(ydiff, zdiff));
			glm::f32vec3 centroid(
				(extents.d[0][0] + extents.d[0][1]),
				(extents.d[1][0] + extents.d[1][1]),
				(extents.d[2][0] + extents.d[2][1]));
			bounds[0] = (glm::f32vec3(centroid) - glm::f32vec3(dim)) * 0.5f;
			bounds[1] = (glm::f32vec3(centroid) + glm::f32vec3(dim)) * 0.5f;
			root = new OctreeNode;
		}
		void insert(const Extents *extents)
		{
			insert(root, extents, bounds[0], bounds[1], 0);
		}
		void build()
		{
			build(root, bounds[0], bounds[1]);
		}
		~Octree() { delete root; }
		struct QueueElement
		{
			const OctreeNode *node; // octree node held by this node in the tree 
			float t; // used as key 
			QueueElement(const OctreeNode *n, float thit) : node(n), t(thit) {}
			// comparator is > instead of < so priority_queue behaves like a min-heap
			friend bool operator < (const QueueElement &a, const QueueElement &b) { return a.t > b.t; }
		};
		glm::f32vec3 bounds[2];
		OctreeNode *root;
	private:
		void insert(
			OctreeNode *node, const Extents *extents,
			glm::f32vec3 boundMin, glm::f32vec3 boundMax, int depth)
		{
			if (node->isLeaf) {
				if (node->data.size() == 0 || depth == 16) {
					node->data.push_back(extents);
				}
				else {
					node->isLeaf = false;
					while (node->data.size()) {
						insert(node, node->data.back(), boundMin, boundMax, depth);
						node->data.pop_back();
					}
					insert(node, extents, boundMin, boundMax, depth);
				}
			}
			else {
				// insert bounding volume in the right octree cell
				glm::f32vec3 extentsCentroid = (
					glm::f32vec3(extents->d[0][0], extents->d[1][0], extents->d[2][0]) +
					glm::f32vec3(extents->d[0][1], extents->d[1][1], extents->d[2][1])) * 0.5f;
				glm::f32vec3 nodeCentroid = (boundMax + boundMin) * 0.5f;
				uint8_t childIndex = 0;
				if (extentsCentroid[0] > nodeCentroid[0]) childIndex += 4;
				if (extentsCentroid[1] > nodeCentroid[1]) childIndex += 2;
				if (extentsCentroid[2] > nodeCentroid[2]) childIndex += 1;
				glm::f32vec3 childBoundMin, childBoundMax;
				glm::f32vec3 boundCentroid = (boundMin + boundMax) * 0.5f;
				computeChildBound(childIndex, boundCentroid, boundMin, boundMax, childBoundMin, childBoundMax);
				if (node->child[childIndex] == NULL)
					node->child[childIndex] = new OctreeNode, node->child[childIndex]->depth = depth;
				insert(node->child[childIndex], extents, childBoundMin, childBoundMax, depth + 1);
			}
		}
		void computeChildBound(
			const uint8_t &i, const glm::f32vec3 &boundCentroid,
			const glm::f32vec3 &boundMin, const glm::f32vec3 &boundMax,
			glm::f32vec3 &pMin, glm::f32vec3 &pMax) const
		{
			pMin[0] = (i & 4) ? boundCentroid[0] : boundMin[0];
			pMax[0] = (i & 4) ? boundMax[0] : boundCentroid[0];
			pMin[1] = (i & 2) ? boundCentroid[1] : boundMin[1];
			pMax[1] = (i & 2) ? boundMax[1] : boundCentroid[1];
			pMin[2] = (i & 1) ? boundCentroid[2] : boundMin[2];
			pMax[2] = (i & 1) ? boundMax[2] : boundCentroid[2];
		}
		// bottom-up construction
		void build(OctreeNode *node, const glm::f32vec3 &boundMin, const glm::f32vec3 &boundMax)
		{
			if (node->isLeaf) {
				// compute leaf node bounding volume
				for (uint32_t i = 0; i < node->data.size(); ++i) {
					node->extents.extendBy(*node->data[i]);
				}
			}
			else {
				for (uint8_t i = 0; i < 8; ++i)
					if (node->child[i]) {
						glm::f32vec3 childBoundMin, childBoundMax;
						glm::f32vec3 boundCentroid = (boundMin + boundMax) * 0.5f;
						computeChildBound(i, boundCentroid, boundMin, boundMax, childBoundMin, childBoundMax);
						build(node->child[i], childBoundMin, childBoundMax);
						node->extents.extendBy(node->child[i]->extents);
					}
			}
		}
	};

Octree *octree;
	
public:

	BVH(std::vector<std::unique_ptr<const RT_Mesh>>& mesh_list);
	bool intersect(Ray* ray, float& t_near, Ray::Hitinfo& info) const;
	~BVH();

	static unsigned long long getVolumeTestCount() {
		return num_ray_volume_tests;
	}

};

