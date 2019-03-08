#include "BBoxAcceleration.h"



BBoxAcceleration::BBoxAcceleration(std::vector<std::unique_ptr<const RT_Mesh>>& m) : AccelerationStructure(m) {};

const RT_Mesh* BBoxAcceleration::intersect(Ray* ray, float& tHit, Ray::Hitinfo& info) const
{
	const RT_Mesh* hit_mesh = nullptr;
	//const glm::vec3 invDir = 1 / ray->direction;
//	const bool sign[] = { ray->direction.x < 0, ray->direction.y < 0, ray->direction.z < 0 };
	float t = tHit;
	float t2 = tHit;
	//I use intput T as maximum distance to search, therefore i must use tHit again otherwise I'm gonna miss the triangles inside bboxes
	for (const auto& mesh : meshes) {
		// If you intersect the box
		if(ray->intersectBB((mesh->boundary_points), t2)) {
			// Then test if the ray intersects the mesh and if does then first check
			// if the intersection distance is the nearest and if we pass that test as well
			// then update tNear variable with t and keep a pointer to the intersected mesh
			//Ray::Hitinfo info_current;
			if (mesh->intersect(ray, t, info) && t < tHit) {
				tHit = t;
				hit_mesh = mesh.get();
				//info_current = info;
			}
		}
	}

	// Return true if the variable intersectedMesh is not null, false otherwise
	return hit_mesh;
}

BBoxAcceleration::~BBoxAcceleration()
{
}
