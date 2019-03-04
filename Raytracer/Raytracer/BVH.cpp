#include "BVH.h"


BVH::BVH(std::vector<std::unique_ptr<const RT_Mesh>>& mesh_list) : 
	AccelerationStructure(mesh_list),
	extents(NULL),
	octree(NULL)
{
	Extents sceneExtents;
	extents = new Extents[mesh_list.size()];
	for (uint32_t i = 0; i < mesh_list.size(); ++i) {
		for (uint8_t j = 0; j < plane_count; ++j) {
			mesh_list[i]->computeBounds(planeSetNormals[j], extents[i].d[j][0], extents[i].d[j][1]);
		}
		extents[i].mesh = mesh_list[i].get();
		sceneExtents.extendBy(extents[i]);
	}
	// create hierarchy                                                                                                                                                                                     
	octree = new Octree(sceneExtents);
	for (uint32_t i = 0; i < mesh_list.size(); ++i) {
		octree->insert(extents + i);
	}
	octree->build();
}

const glm::f32vec3 BVH::planeSetNormals[BVH::plane_count] = {
	glm::f32vec3(1, 0, 0),
	glm::f32vec3(0, 1, 0),
	glm::f32vec3(0, 0, 1),
	glm::f32vec3(sqrtf(3) / 3.f,  sqrtf(3) / 3.f, sqrtf(3) / 3.f),
	glm::f32vec3(-sqrtf(3) / 3.f,  sqrtf(3) / 3.f, sqrtf(3) / 3.f),
	glm::f32vec3(-sqrtf(3) / 3.f, -sqrtf(3) / 3.f, sqrtf(3) / 3.f),
	glm::f32vec3(sqrtf(3) / 3.f, -sqrtf(3) / 3.f, sqrtf(3) / 3.f) 
};

inline bool BVH::Extents::intersect(
	const float *precomputedNumerator, const float *precomputeDenominator,
	float &tNear, float &tFar, uint8_t &planeIndex)
{
	std::atomic_fetch_add(&num_ray_volume_tests, 1);
	//__sync_fetch_and_add(&numRayVolumeTests, 1);
	for (uint8_t i = 0; i < plane_count; ++i) {
		float tn = (d[i][0] - precomputedNumerator[i]) / precomputeDenominator[i];
		float tf = (d[i][1] - precomputedNumerator[i]) / precomputeDenominator[i];
		if (precomputeDenominator[i] < 0) std::swap(tn, tf);
		if (tn > tNear) tNear = tn, planeIndex = i;
		if (tf < tFar) tFar = tf;
		if (tNear > tFar) return false; // test for an early stop 
	}

	return true;
}

const RT_Mesh* BVH::intersect(Ray* ray, float& t_near, Ray::Hitinfo& info) const
{
	const RT_Mesh *hitObject = NULL;
	float precomputedNumerator[BVH::plane_count], precomputeDenominator[BVH::plane_count];
	for (uint8_t i = 0; i < plane_count; ++i) {
		precomputedNumerator[i] = glm::dot(planeSetNormals[i], ray->origin);
		precomputeDenominator[i] = glm::dot(planeSetNormals[i], ray->direction);
	}
#if 0 
/*	float tClosest = ray.tmax;
	for (uint32_t i = 0; i < rc->objects.size(); ++i) {
		__sync_fetch_and_add(&numRayVolumeTests, 1);
		float tNear = -kInfinity, tFar = kInfinity;
		uint8_t planeIndex;
		if (extents[i].intersect(precomputedNumerator, precomputeDenominator, tNear, tFar, planeIndex)) {
			IsectData isectDataCurrent;
			if (rc->objects[i]->intersect(ray, isectDataCurrent)) {
				if (isectDataCurrent.t < tClosest && isectDataCurrent.t > ray.tmin) {
					isectData = isectDataCurrent;
					hitObject = rc->objects[i];
					tClosest = isectDataCurrent.t;
				}
			}
		}
	}*/
#else 
	uint8_t planeIndex = 0;
	float tNear = 0, tFar = ray->tmax;
	if (!octree->root->extents.intersect(precomputedNumerator, precomputeDenominator, tNear, tFar, planeIndex)
		|| tFar < 0 || tNear > ray->tmax)
		return NULL;
	OUT float t;
	float tMin = t = tFar;
	std::priority_queue<BVH::Octree::QueueElement> queue;
	queue.push(BVH::Octree::QueueElement(octree->root, 0));
	while (!queue.empty() && queue.top().t < tMin) {
		const OctreeNode *node = queue.top().node;
		queue.pop();
		if (node->isLeaf) {
			for (uint32_t i = 0; i < node->data.size(); ++i) {
				Ray::Hitinfo info_current;
				if (node->data[i]->mesh->intersect(ray,OUT t, info_current)) {
					if (t < tMin) {
						tMin = t;///Somehow too much Ts...redundant? no time to think
						hitObject = node->data[i]->mesh;
						info = info_current;
					}
				}
			}
		}
		else {
			for (uint8_t i = 0; i < 8; ++i) {
				if (node->child[i] != NULL) {
					float tNearChild = 0, tFarChild = tFar;
					if (node->child[i]->extents.intersect(precomputedNumerator, precomputeDenominator,
						tNearChild, tFarChild, planeIndex)) {
						float t = (tNearChild < 0 && tFarChild >= 0) ? tFarChild : tNearChild;
						queue.push(BVH::Octree::QueueElement(node->child[i], t));
					}
				}
			}
		}
	}
#endif 

	return hitObject;
}

BVH::~BVH()
{
}

