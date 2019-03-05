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

//static const float inf = std::numeric_limits<float>::max();
/*
*class taken from https://www.scratchapixel.com/lessons/advanced-rendering/introduction-acceleration-structure/bounding-volume-hierarchy-BVH-part2
*Authors of www.sratchapixel.com are unknown
*/

class BBox
{
public:
	BBox() {}
	BBox(glm::vec3 min_, glm::vec3 max_)
	{
		bounds[0] = min_;
		bounds[1] = max_;
	}
	BBox& extendBy(const glm::vec3& p)
	{
		if (p.x < bounds[0].x) bounds[0].x = p.x;
		if (p.y < bounds[0].y) bounds[0].y = p.y;
		if (p.z < bounds[0].z) bounds[0].z = p.z;
		if (p.x > bounds[1].x) bounds[1].x = p.x;
		if (p.y > bounds[1].y) bounds[1].y = p.y;
		if (p.z > bounds[1].z) bounds[1].z = p.z;

		return *this;
	}
	/*inline */glm::vec3 centroid() const { return (bounds[0] + bounds[1]) * 0.5f; }
	glm::vec3& operator [] (bool i) { return bounds[i]; }
	const glm::vec3 operator [] (bool i) const { return bounds[i]; }
	const bool intersect(const glm::vec3& orig, const glm::vec3& invDir, const bool* sign, float& tHit) const
	{
		std::atomic_fetch_add(&AccelerationStructure::box_test_count, 1);
		float tmin, tmax, tymin, tymax, tzmin, tzmax;

		tmin = (bounds[sign[0]].x - orig.x) * invDir.x;
		tmax = (bounds[1 - sign[0]].x - orig.x) * invDir.x;
		tymin = (bounds[sign[1]].y - orig.y) * invDir.y;
		tymax = (bounds[1 - sign[1]].y - orig.y) * invDir.y;

		if ((tmin > tymax) || (tymin > tmax))
			return false;

		if (tymin > tmin)
			tmin = tymin;
		if (tymax < tmax)
			tmax = tymax;

		tzmin = (bounds[sign[2]].z - orig.z) * invDir.z;
		tzmax = (bounds[1 - sign[2]].z - orig.z) * invDir.z;

		if ((tmin > tzmax) || (tzmin > tmax))
			return false;

		if (tzmin > tmin)
			tmin = tzmin;
		if (tzmax < tmax)
			tmax = tzmax;

		tHit = tmin;

		return true;
	}
	glm::vec3 bounds[2] = { glm::vec3(inf), glm::vec3(-inf) };
};

//const bool BBox::intersect(const glm::vec3& orig, const glm::vec3& invDir, const bool* sign, float& tHit) const
/*{
	std::atomic_fetch_add(&AccelerationStructure::box_test_count, 1);
	float tmin, tmax, tymin, tymax, tzmin, tzmax;

	tmin = (bounds[sign[0]].x - orig.x) * invDir.x;
	tmax = (bounds[1 - sign[0]].x - orig.x) * invDir.x;
	tymin = (bounds[sign[1]].y - orig.y) * invDir.y;
	tymax = (bounds[1 - sign[1]].y - orig.y) * invDir.y;

	if ((tmin > tymax) || (tymin > tmax))
		return false;

	if (tymin > tmin)
		tmin = tymin;
	if (tymax < tmax)
		tmax = tymax;

	tzmin = (bounds[sign[2]].z - orig.z) * invDir.z;
	tzmax = (bounds[1 - sign[2]].z - orig.z) * invDir.z;

	if ((tmin > tzmax) || (tzmin > tmax))
		return false;

	if (tzmin > tmin)
		tmin = tzmin;
	if (tzmax < tmax)
		tmax = tzmax;

	tHit = tmin;

	return true;
}*/

class BVH : public AccelerationStructure
{
	static const uint8_t plane_count = 7;
	static const glm::f32vec3 planeSetNormals[plane_count];
	struct Extents
	{
		Extents() {
			for (uint8_t i = 0; i < plane_count; ++i)
				d[i][0] = inf, d[i][1] = -inf;
		}
		void extendBy(const Extents &extents);
		glm::f32vec3 centroid() const;
		bool intersect(
			const float *precomputedNumerator, const float *precomputeDenominator,
			float &tNear, float &tFar, uint8_t &planeIndex) 
const;
		float d[plane_count][2]; // d values for each plane-set normals 
		const RT_Mesh* mesh; // pointer contained by the volume (used by octree) 
	};

	struct Octree
	{
		Octree(const Extents& scene_extents);
		~Octree();
		void insert(const Extents* extents);
		void build();

		struct OctreeNode
		{
			OctreeNode *child[8] = { nullptr };
			std::vector<const Extents *> node_extents_list;
			Extents node_extents;
			bool isLeaf = true;
			//uint8_t depth; // just for debugging 
			//OctreeNode() : isLeaf(true) { memset(child, 0x0, sizeof(OctreeNode *) * 8); }
			//~OctreeNode() { for (uint8_t i = 0; i < 8; ++i) if (child[i] != NULL) delete child[i]; }
		};
	
		struct QueueElement {
			const OctreeNode* node;
			float t;
			QueueElement(const OctreeNode* n, float tn): node(n),t(tn){}
			friend bool operator < (const QueueElement &a, const QueueElement &b) { return a.t > b.t; }//prioritni fronta se chova jako halda podle min.
		};

		OctreeNode* root = nullptr;
		BBox bbox;

	private:
		void deleteOctreeNode(OctreeNode*& node);
		void insert(
			OctreeNode *node, const Extents *extents, const BBox& bbox,
			uint32_t depth);
		//void computeChildBound(
			//const uint8_t &i, const glm::f32vec3 &boundCentroid,
			//const glm::f32vec3 &boundMin, const glm::f32vec3 &boundMax,
			//glm::f32vec3 &pMin, glm::f32vec3 &pMax) const
		//{
			//pMin[0] = (i & 4) ? boundCentroid[0] : boundMin[0];
			//pMax[0] = (i & 4) ? boundMax[0] : boundCentroid[0];
			//pMin[1] = (i & 2) ? boundCentroid[1] : boundMin[1];
			//pMax[1] = (i & 2) ? boundMax[1] : boundCentroid[1];
			//pMin[2] = (i & 1) ? boundCentroid[2] : boundMin[2];
			//pMax[2] = (i & 1) ? boundMax[2] : boundCentroid[2];
		//}
		// bottom-up construction
		void build(OctreeNode *node, const BBox& bbox);
	};

	Octree *octree = nullptr;
	std::vector<Extents> extents_list;
	
public:
	BVH(std::vector<std::unique_ptr<const RT_Mesh>>& mesh_list);
	const RT_Mesh* intersect(Ray* ray, float& t_near, Ray::Hitinfo& info) const;
	~BVH();
};