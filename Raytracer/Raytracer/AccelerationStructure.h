#pragma once
#include <atomic>
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
	static unsigned long long getVolumeTestCount() {return num_ray_volume_tests;}
	static unsigned long long getBoxTestCount() {return box_test_count;}
	static unsigned long long getTrianglesIntersected() { return triangles_intersected; }
//protected:
	static std::atomic<unsigned long long> num_ray_volume_tests;
	static std::atomic<unsigned long long> box_test_count;
	static std::atomic<unsigned long long> triangles_intersected;
	const std::vector<std::unique_ptr<const RT_Mesh>> meshes;
};

