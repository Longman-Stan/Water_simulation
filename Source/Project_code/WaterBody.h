#pragma once

#include <include/glm.h>
#include <Core/GPU/Mesh.h>
#include <Core/GPU/Shader.h>
#include <vector>
#include <string>
#include "WaterPatch.h"

class WaterBody {

public:
	WaterBody();
	WaterBody(int width_patch, int height_patch, float resolution_patch, int num_patches_width, int num_patches_height);

	int num_patches_width;
	int num_patches_height;
	int width_patch;
	int height_patch;
	int resolution_patch;
	std::vector<WaterPatch> patches;
};