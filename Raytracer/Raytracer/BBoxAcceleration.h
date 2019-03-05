#pragma once
#include <vector>
#include "AccelerationStructure.h"

class BBoxAcceleration : public AccelerationStructure
{
public:
	BBoxAcceleration(std::vector<std::unique_ptr<const RT_Mesh>>& m);

	virtual const RT_Mesh* intersect(Ray* ray, float& tHit, Ray::Hitinfo& info) const;

	~BBoxAcceleration();
};

