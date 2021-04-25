#pragma once

#include <include/glm.h>
#include <Core/GPU/Mesh.h>

class WaterPatch {

public:
	WaterPatch();
	WaterPatch(std::string name, int offsetX, int offsetZ, int width, int height, float resolution);
	Mesh* getMesh();
private:
	Mesh* mesh_patch;
};