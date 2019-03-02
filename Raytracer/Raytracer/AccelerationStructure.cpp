#include "AccelerationStructure.h"

AccelerationStructure::AccelerationStructure(std::vector<std::unique_ptr<const RT_Mesh>>& m) : meshes(std::move(m)) {}

bool AccelerationStructure::intersect(Ray* ray, float& tHit) const
{
	float t = inf;
	uint32_t triangle_count;
	const RT_Mesh* hit_mesh = nullptr;

	for (const auto& mesh : meshes)
	{
		if (mesh->intersect(ray, t) && t < tHit) {
			hit_mesh = mesh.get();
			tHit = t;		
		}
	}
	return (hit_mesh != nullptr);
}
