#include "WaterBody.h"

WaterBody::WaterBody()
{
	patches.push_back(WaterPatch());
}

WaterBody::WaterBody(int width_patch, int height_patch, float resolution_patch, int num_patches_width, int num_patches_height)
{
	width_patch = width_patch;
	height_patch = height_patch;
	resolution_patch = resolution_patch;
	num_patches_width = num_patches_width;
	num_patches_height = num_patches_height;

	std::string base_name = "patch";

	float baseX = -width_patch * num_patches_width / 2;
	float baseZ = -height_patch * num_patches_height / 2;
	
	for (int i = 0; i < num_patches_width; i++)
		for (int j = 0; j < num_patches_height; j++)
		{
			patches.push_back(WaterPatch(base_name + std::to_string(i) + std::to_string(j), baseX + i*width_patch, 
											baseZ + j*height_patch, width_patch, height_patch, resolution_patch) );
		}
}