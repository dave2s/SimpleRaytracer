#pragma once
#include <vector>
#include <memory>
#include "RT_Mesh.h"
class AccelerationStructure
{
public:
	AccelerationStructure(std::vector<std::unique_ptr<const RT_Mesh>>& m) : meshes(std::move(m)) {}
	virtual ~AccelerationStructure() {}
	virtual bool intersect(const glm::f32vec3& orig, const glm::f32vec3& dir, const uint32_t& rayId, float& tHit) const
	{
		const RT_Mesh* intersectedMesh = nullptr;
		float t = inf;
		for (const auto& mesh : meshes) {
			if (mesh->intersect(orig, dir, t) && t < tHit) {
				intersectedMesh = mesh.get();
				tHit = t;
			}
		}

		return (intersectedMesh != nullptr);
	}
protected:
	const std::vector<std::unique_ptr<const RT_Mesh>> meshes;
};

