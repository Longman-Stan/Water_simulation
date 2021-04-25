#include "WaterPatch.h"

Mesh* constructPatch(std::string patch_name, int offsetX, int offsetZ, int width, int height, float resolution)
{
	std::vector<VertexFormat> vertices;

	int vertices_width = width / resolution + 1;
	int vertices_height = height / resolution + 1;

	glm::vec3 base_position = glm::vec3(offsetX, 0, offsetZ);
	int i, j;
	for (i = 0; i < vertices_height; i++)
		for (j = 0; j < vertices_width; j++)
		{
			vertices.push_back(VertexFormat(base_position + glm::vec3((float)j * resolution, 0, (float)i * resolution),glm::vec3(1.f,1.f,1.f),glm::vec3(1.f,1.f,1.f),
				glm::vec2(i/(vertices_height-1),j/(vertices_width-1))));
		}

	for (i = 0; i < vertices.size(); i++)
	{
		glm::vec3 aux = vertices[i].position - base_position;
		vertices[i].text_coord = glm::vec2(((float)aux.x) / width, ((float)aux.z) / height);
	}

	std::vector<unsigned short> indices;
	for (i = 0; i < vertices_height - 1; i++)
		for (j = 0; j < vertices_width - 1; j++)
		{
			int base = i * vertices_width + j;
			indices.push_back(base); indices.push_back(base + vertices_width);  indices.push_back(base + vertices_width + 1);
			indices.push_back(base); indices.push_back(base + vertices_width + 1); indices.push_back(base + 1);
		}

	Mesh* patch = new Mesh(patch_name);
	patch->InitFromData(vertices, indices);

	return patch;
}


WaterPatch::WaterPatch()
{
	mesh_patch = constructPatch("default_water_patch", -50, -50, 100, 100, 1);
}

WaterPatch::WaterPatch(std::string name, int offsetX, int offsetZ, int width, int height, float resolution)
{
	mesh_patch = constructPatch(name, offsetX, offsetZ, width, height, resolution);
}

Mesh* WaterPatch::getMesh()
{
	return mesh_patch;
}