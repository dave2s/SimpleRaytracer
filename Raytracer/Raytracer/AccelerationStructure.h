#pragma once
#include <vector>
#include <memory>
#include "RT_Mesh.h"
#include "Ray.h"
class AccelerationStructure
{
public:
	AccelerationStructure(std::vector<std::unique_ptr<const RT_Mesh>>& m);
	virtual ~AccelerationStructure();
	virtual const RT_Mesh* intersect(Ray* ray, float& tHit, Ray::Hitinfo& inf) const;
protected:
	const std::vector<std::unique_ptr<const RT_Mesh>> meshes;
};

