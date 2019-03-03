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
	virtual bool intersect(Ray* ray, float& tHit, Ray::Hitinfo& info, const RT_Mesh* m) const;
protected:
	const std::vector<std::unique_ptr<const RT_Mesh>> meshes;
};

