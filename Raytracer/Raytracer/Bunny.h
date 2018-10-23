#pragma once
class Bunny
{
public:
	Bunny();
	~Bunny();

	unsigned int vertex_count = 453;
	unsigned int index_count = 948;

	const float* vertices;
	const unsigned int* indices;
};

