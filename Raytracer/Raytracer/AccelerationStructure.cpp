#include "AccelerationStructure.h"

AccelerationStructure::AccelerationStructure(std::vector<std::unique_ptr<const RT_Mesh*>>& m) : meshes(std::move(m)) {}

bool AccelerationStructure::intersect(const glm::f32vec3& orig, const glm::f32vec3& dir, const uint32_t& rayId, float& tHit) const
{
	const RT_Mesh* intersectedMesh = nullptr;
	float t = inf;
	for (const auto& mesh : meshes) {
		if ((*mesh)->intersect(orig, dir, t) && t < tHit) {
			intersectedMesh = (*mesh);
			tHit = t;
		}
	}

	return (intersectedMesh != nullptr);
}
