#include "AccelerationStructure.h"

AccelerationStructure::AccelerationStructure(std::vector<std::unique_ptr<const RT_Mesh>>& m) : meshes(std::move(m)) {}

bool AccelerationStructure::intersect(Ray* ray, float& tHit, Ray::Hitinfo& info, const RT_Mesh* mesh) const
{
	float t = inf;
	uint32_t triangle_count;
	const RT_Mesh* hit_mesh = nullptr;

	for (const auto& m : meshes)
	{
		if (m->intersect(ray, t, info) && t < tHit) {
			hit_mesh = m.get();
			tHit = t;		
		}
	}
	mesh = hit_mesh;
	return (hit_mesh != nullptr);
}

AccelerationStructure::~AccelerationStructure() {}
