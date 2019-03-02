#pragma once
#include <vector>
#include <memory>
#include "RT_Mesh.h"
class AccelerationStructure
{
public:
	AccelerationStructure(std::vector<std::unique_ptr<const RT_Mesh*>>& m);
	virtual ~AccelerationStructure();
	virtual bool intersect(const glm::f32vec3& orig, const glm::f32vec3& dir, const uint32_t& rayId, float& tHit) const;
protected:
	const std::vector<std::unique_ptr<const RT_Mesh*>> meshes;
};

