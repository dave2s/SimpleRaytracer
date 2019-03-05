#include "BVH.h"
/*
BVH::Extents::Extents()
{
	for (uint8_t i = 0; i < plane_count; ++i) {
		d[i][0] = inf; d[i][1] = -inf;
	}
}*/

void BVH::Extents::extendBy(const Extents & extents)
{
	for (uint8_t i = 0; i < plane_count; ++i) {
		if (extents.d[i][0] < d[i][0]) d[i][0] = extents.d[i][0];
		if (extents.d[i][1] > d[i][1]) d[i][1] = extents.d[i][1];
	}
}

glm::f32vec3 BVH::Extents::centroid() const
{
	return glm::f32vec3(
	d[0][0] + d[0][1] * 0.5f,
	d[1][0] + d[1][1] * 0.5f,
	d[2][0] + d[2][1] * 0.5f);
}

BVH::~BVH()
{
	delete octree;
}

BVH::Octree::Octree(const Extents & extents)
{
	float xdiff = extents.d[0][1] - extents.d[0][0];
	float ydiff = extents.d[1][1] - extents.d[1][0];
	float zdiff = extents.d[2][1] - extents.d[2][0];
	float dim = std::max(xdiff, std::max(ydiff, zdiff));
	glm::f32vec3 centroid(
		(extents.d[0][0] + extents.d[0][1]),
		(extents.d[1][0] + extents.d[1][1]),
		(extents.d[2][0] + extents.d[2][1]));
	bbox[0] = (glm::f32vec3(centroid) - glm::f32vec3(dim)) * 0.5f;
	bbox[1] = (glm::f32vec3(centroid) + glm::f32vec3(dim)) * 0.5f;
	root = new OctreeNode;
}

BVH::Octree::~Octree()
{
	deleteOctreeNode(root);
}

void BVH::Octree::insert(const Extents * extents)
{
	insert(root, extents, bbox, 0);
}

void BVH::Octree::build()
{
	build(root, bbox);
}

void BVH::Octree::deleteOctreeNode(OctreeNode *& node)
{
	for (uint8_t i = 0; i < 8; i++) {
		if (node->child[i] != nullptr) {
			deleteOctreeNode(node->child[i]);
		}
	}
	delete node;
}

void BVH::Octree::insert(OctreeNode * node, const Extents * extents, const BBox& bbox, uint32_t depth)
{
	if (node->isLeaf) {
		if (node->node_extents_list.size() == 0 || depth == 16) {
			node->node_extents_list.push_back(extents);
		}
		else {
			node->isLeaf = false;
			// Re-insert extents held by this node
			while (node->node_extents_list.size()) {
				insert(node, node->node_extents_list.back(), bbox, depth);
				node->node_extents_list.pop_back();
			}
			// Insert new extent
			insert(node, extents, bbox, depth);
		}
	}
	else {
		// Need to compute in which child of the current node this extents should
		// be inserted into
		glm::f32vec3 extentsCentroid = extents->centroid();
		glm::f32vec3 nodeCentroid = (bbox[0] + bbox[1]) * 0.5f;
		BBox childBBox;
		uint8_t childIndex = 0;
		// x-axis
		if (extentsCentroid.x > nodeCentroid.x) {
			childIndex = 4;
			childBBox[0].x = nodeCentroid.x;
			childBBox[1].x = bbox[1].x;
		}
		else {
			childBBox[0].x = bbox[0].x;
			childBBox[1].x = nodeCentroid.x;
		}
		// y-axis
		if (extentsCentroid.y > nodeCentroid.y) {
			childIndex += 2;
			childBBox[0].y = nodeCentroid.y;
			childBBox[1].y = bbox[1].y;
		}
		else {
			childBBox[0].y = bbox[0].y;
			childBBox[1].y = nodeCentroid.y;
		}
		// z-axis
		if (extentsCentroid.z > nodeCentroid.z) {
			childIndex += 1;
			childBBox[0].z = nodeCentroid.z;
			childBBox[1].z = bbox[1].z;
		}
		else {
			childBBox[0].z = bbox[0].z;
			childBBox[1].z = nodeCentroid.z;
		}

		// Create the child node if it doesn't exsit yet and then insert the extents in it
		if (node->child[childIndex] == nullptr)
			node->child[childIndex] = new OctreeNode;
		insert(node->child[childIndex], extents, childBBox, depth + 1);
	}
}

void BVH::Octree::build(OctreeNode * node, const BBox& bbox)
{
	if (node->isLeaf) {
		for (const auto& e : node->node_extents_list) {
			node->node_extents.extendBy(*e);
		}
	}
	else {
		for (uint8_t i = 0; i < 8; ++i) {
			if (node->child[i]) {
				BBox childBBox;
				glm::f32vec3 centroid = bbox.centroid();
				// x-axis
				childBBox[0].x = (i & 4) ? centroid.x : bbox[0].x;
				childBBox[1].x = (i & 4) ? bbox[1].x : centroid.x;
				// y-axis
				childBBox[0].y = (i & 2) ? centroid.y : bbox[0].y;
				childBBox[1].y = (i & 2) ? bbox[1].y : centroid.y;
				// z-axis
				childBBox[0].z = (i & 1) ? centroid.z : bbox[0].z;
				childBBox[1].z = (i & 1) ? bbox[1].z : centroid.z;

				// Inspect child
				build(node->child[i], childBBox);

				// Expand extents with extents of child
				node->node_extents.extendBy(node->child[i]->node_extents);
			}
		}
	}
}

const glm::f32vec3 BVH::planeSetNormals[BVH::plane_count]{
glm::f32vec3(1, 0, 0),
	glm::f32vec3(0, 1, 0),
	glm::f32vec3(0, 0, 1),
	glm::f32vec3(sqrtf(3) / 3.f,  sqrtf(3) / 3.f, sqrtf(3) / 3.f),
	glm::f32vec3(-sqrtf(3) / 3.f,  sqrtf(3) / 3.f, sqrtf(3) / 3.f),
	glm::f32vec3(-sqrtf(3) / 3.f, -sqrtf(3) / 3.f, sqrtf(3) / 3.f),
	glm::f32vec3(sqrtf(3) / 3.f, -sqrtf(3) / 3.f, sqrtf(3) / 3.f)
};

BVH::BVH(std::vector<std::unique_ptr<const RT_Mesh>>& m) : AccelerationStructure(m)
{
	Extents scene_extents;
	extents_list.resize(meshes.size());
	//std::vector<BVH::Extents> extents_list();

	uint32_t total_num_triangles = 0;

	for (uint32_t i = 0; i < meshes.size(); ++i) {
		for (uint8_t j = 0; j < plane_count; ++j) {
			for (const auto vertex : meshes[i]->_vertices) {
				float d = glm::dot(planeSetNormals[j], vertex.position);
				// set dNEar and dFar
				if (d < extents_list[i].d[j][0]) extents_list[i].d[j][0] = d;
				if (d > extents_list[i].d[j][1]) extents_list[i].d[j][1] = d;
			}
		}
		scene_extents.extendBy(extents_list[i]); // rozsir rozsah sceny objektu
		extents_list[i].mesh = meshes[i].get(); // extent uklada ukazatel na objekt ktery obsahuje
	}
	// Mame rozsah sceny, postavime octree
	octree = new Octree(scene_extents);

	for (uint32_t i = 0; i < meshes.size(); ++i) {
		octree->insert(&extents_list[i]);
	}
	//bottom-up
	octree->build();
}

bool BVH::Extents::intersect(
	const float* precomputedNumerator,
	const float* precomputedDenominator,
	float& tNear,
	float& tFar,
	uint8_t& planeIndex) const
{
	std::atomic_fetch_add(&num_ray_volume_tests, 1);
	for (uint8_t i = 0; i < plane_count; ++i) {
		float tNearExtents = (d[i][0] - precomputedNumerator[i]) / precomputedDenominator[i];
		float tFarExtents = (d[i][1] - precomputedNumerator[i]) / precomputedDenominator[i];
		if (precomputedDenominator[i] < 0) std::swap(tNearExtents, tFarExtents);
		if (tNearExtents > tNear) tNear = tNearExtents, planeIndex = i;
		if (tFarExtents < tFar) tFar = tFarExtents;
		if (tNear > tFar) return false;
	}

	return true;
}

const RT_Mesh* BVH::intersect(Ray* ray, float& tHit, Ray::Hitinfo& info) const
{
	tHit = inf;
	const RT_Mesh* hit_mesh = nullptr;
	float precomputedNumerator[BVH::plane_count];
	float precomputedDenominator[BVH::plane_count];
	for (uint8_t i = 0; i < plane_count; ++i) {
		precomputedNumerator[i] = glm::dot(planeSetNormals[i], ray->origin);
		precomputedDenominator[i] = glm::dot(planeSetNormals[i], ray->direction);
	}

	/*
	tNear = kInfinity; // set
	for (uint32_t i = 0; i < meshes.size(); ++i) {
		numRayVolumeTests++;
		float tn = -kInfinity, tf = kInfinity;
		uint8_t planeIndex;
		if (extents[i].intersect(precomputedNumerator, precomputedDenominator, tn, tf, planeIndex)) {
			if (tn < tNear) {
				intersectedMesh = meshes[i].get();
				tNear = tn;
				// normal = planeSetNormals[planeIndex];
			}
		}
	}
	*/

	uint8_t planeIndex;
	float tNear = 0, tFar = inf; // tNear, tFar for the intersected extents 
	if (!octree->root->node_extents.intersect(precomputedNumerator, precomputedDenominator, tNear, tFar, planeIndex) || tFar < 0)
		return false;
	tHit = tFar;
	std::priority_queue<BVH::Octree::QueueElement> queue;
	queue.push(BVH::Octree::QueueElement(octree->root, 0));
	while (!queue.empty() && queue.top().t < tHit) {
		const Octree::OctreeNode *node = queue.top().node;
		queue.pop();
		if (node->isLeaf) {
			for (const auto& e : node->node_extents_list) {
				float t = inf;
				if (e->mesh->intersect(ray,t,info) && t < tHit) {
					tHit = t;
					hit_mesh = e->mesh;
				}
			}
		}
		else {
			for (uint8_t i = 0; i < 8; ++i) {
				if (node->child[i] != nullptr) {
					float t_near_child = 0, t_far_child = tFar;
					if (node->child[i]->node_extents.intersect(precomputedNumerator, precomputedDenominator, t_near_child, t_far_child, planeIndex)) {
						float t = (t_near_child < 0 && t_far_child >= 0) ? t_far_child : t_near_child;
						queue.push(BVH::Octree::QueueElement(node->child[i], t));
					}
				}
			}
		}
	}

	return hit_mesh;
}
