#include "Laborator6.h"

#include <vector>
#include <iostream>

#include <Core/Engine.h>

using namespace std;

#define rand01 (rand() / static_cast<float>(RAND_MAX))

// Order of function calling can be seen in "Source/Core/World.cpp::LoopUpdate()"
// https://github.com/UPB-Graphics/SPG-Framework/blob/master/Source/Core/World.cpp

Laborator6::Laborator6()
{
}

Laborator6::~Laborator6()
{

}

void Laborator6::Init()
{
	outputType = 0;

	auto camera = GetSceneCamera();
	camera->SetPositionAndRotation(glm::vec3(0, 2, 3.5), glm::quat(glm::vec3(-20 * TO_RADIANS, 0, 0)));
	camera->Update();

	TextureManager::LoadTexture(RESOURCE_PATH::TEXTURES, "ground.jpg");

	// Load a mesh from file into GPU memory
	{
		Mesh* mesh = new Mesh("box");
		mesh->LoadMesh(RESOURCE_PATH::MODELS + "Primitives", "box.obj");
		meshes[mesh->GetMeshID()] = mesh;
	}

	{
		Mesh* mesh = new Mesh("plane");
		mesh->LoadMesh(RESOURCE_PATH::MODELS + "Primitives", "plane50.obj");
		mesh->UseMaterials(false);
		meshes[mesh->GetMeshID()] = mesh;
	}

	// Load a mesh from file into GPU memory
	{
		Mesh* mesh = new Mesh("sphere");
		mesh->LoadMesh(RESOURCE_PATH::MODELS + "Primitives", "sphere.obj");
		mesh->UseMaterials(false);
		meshes[mesh->GetMeshID()] = mesh;
	}

	{
		Mesh* mesh = new Mesh("quad");
		mesh->LoadMesh(RESOURCE_PATH::MODELS + "Primitives", "quad.obj");
		mesh->UseMaterials(false);
		meshes[mesh->GetMeshID()] = mesh;
	}

	LoadShader("Render2Texture");
	LoadShader("Composition");
	LoadShader("LightPass");

	auto resolution = window->GetResolution();

	frameBuffer = new FrameBuffer();
	frameBuffer->Generate(resolution.x, resolution.y, 3);

	lightBuffer = new FrameBuffer();
	lightBuffer->Generate(resolution.x, resolution.y, 1);

	int gridSize = 3;
	for (int i = -gridSize; i < gridSize; i++)
	{
		for (int j = -gridSize; j < gridSize; j++)
		{
			LightInfo L;
			L.color = glm::vec3(rand01, rand01, rand01);
			L.position = glm::vec3(i, rand01, j) * glm::vec3(3, 2, 3) + glm::vec3(0, 0.5, 0);
			L.radius = rand01 + 3;
			lights.push_back(L);
		}
	}
}

void Laborator6::FrameStart()
{

}

void Laborator6::Update(float deltaTimeSeconds)
{
	ClearScreen();

	// ------------------------------------------------------------------------
	// Deferred rendering pass
	{
		frameBuffer->Bind();

		auto shader = shaders["Render2Texture"];

		TextureManager::GetTexture("default.png")->BindToTextureUnit(GL_TEXTURE0);

		// render scene objects
		RenderMesh(meshes["box"], shader, glm::vec3(1.5, 0.5f, 0), glm::vec3(0.5f));
		RenderMesh(meshes["box"], shader, glm::vec3(0, 1.05f, 0), glm::vec3(2));
		RenderMesh(meshes["box"], shader, glm::vec3(-2, 1.5f, 0));
		RenderMesh(meshes["sphere"], shader, glm::vec3(-4, 1, 1));

		TextureManager::GetTexture("ground.jpg")->BindToTextureUnit(GL_TEXTURE0);
		RenderMesh(meshes["plane"], shader, glm::vec3(0, 0, 0), glm::vec3(0.5f));

		// Render a simple point light bulb for each light (for debugging purposes)
		for (auto &l : lights)
		{
			auto model = glm::translate(glm::mat4(1), l.position);
			model = glm::scale(model, glm::vec3(0.2f));
			RenderMesh(meshes["sphere"], shader, model);
		}
	}

	// ------------------------------------------------------------------------
	// Ligthing pass
	{
		lightBuffer->Bind();

		// Enable buffer color accumulation
		glDepthMask(GL_FALSE);
		glEnable(GL_BLEND);
		glBlendEquation(GL_FUNC_ADD);
		glBlendFunc(GL_ONE, GL_ONE);

		auto shader = shaders["LightPass"];
		shader->Use();

		{
			int texturePositionsLoc = shader->GetUniformLocation("texture_position");
			glUniform1i(texturePositionsLoc, 0);
			frameBuffer->BindTexture(0, GL_TEXTURE0);
		}

		{
			int textureNormalsLoc = shader->GetUniformLocation("texture_normal");
			glUniform1i(textureNormalsLoc, 1);
			frameBuffer->BindTexture(1, GL_TEXTURE0 + 1);
		}

		auto camera = GetSceneCamera();
		glm::vec3 cameraPos = camera->transform->GetWorldPosition();
		int loc_eyePosition = shader->GetUniformLocation("eye_position");
		glUniform3fv(loc_eyePosition, 1, glm::value_ptr(cameraPos));

		auto resolution = window->GetResolution();
		int loc_resolution = shader->GetUniformLocation("resolution");
		glUniform2i(loc_resolution, resolution.x, resolution.y);

		// Face culling for sphere ligth areas
		// Test with both GL_BACK and GL_FRONT and see what's the diference when the camera goes through an area light
		// When GL_BACK is culled the light area will dissapear if the camera enters the light sphere (from interior the sphere is not rendered)
		// When GL_FRONT is culled the light area will always be visible. This is the desired effect.
		// If no culling is active (both GL_BACK and GL_FRONT are rendered) then the light area will double the intensity for each pixel

		// TODO
		glEnable(GL_CULL_FACE);
		glCullFace(GL_FRONT);

		for (auto &l : lights)
		{
			auto model = glm::translate(glm::mat4(1), l.position);
			model = glm::scale(model, glm::vec3(2 * l.radius));
			glUniform1f(shader->loc_light_radius, l.radius);
			glUniform3fv(shader->loc_light_color, 1, glm::value_ptr(l.color));
			glUniform3fv(shader->loc_light_pos, 1, glm::value_ptr(l.position));
			RenderMesh(meshes["sphere"], shader, model);
		}

		// TODO
		glDisable(GL_CULL_FACE);

		glDepthMask(GL_TRUE);
		glDisable(GL_BLEND);
	}

	// ------------------------------------------------------------------------
	// Composition pass
	{
		FrameBuffer::BindDefault();

		auto shader = shaders["Composition"];
		shader->Use();

		int outputTypeLoc = shader->GetUniformLocation("output_type");
		glUniform1i(outputTypeLoc, outputType);

		{
			int texturePositionsLoc = shader->GetUniformLocation("texture_position");
			glUniform1i(texturePositionsLoc, 1);
			frameBuffer->BindTexture(0, GL_TEXTURE0 + 1);
		}

		{
			int textureNormalsLoc = shader->GetUniformLocation("texture_normal");
			glUniform1i(textureNormalsLoc, 2);
			frameBuffer->BindTexture(1, GL_TEXTURE0 + 2);
		}

		{
			int textureColorLoc = shader->GetUniformLocation("texture_color");
			glUniform1i(textureColorLoc, 3);
			frameBuffer->BindTexture(2, GL_TEXTURE0 + 3);
		}

		{
			int textureDepthLoc = shader->GetUniformLocation("texture_depth");
			glUniform1i(textureDepthLoc, 4);
			frameBuffer->BindDepthTexture(GL_TEXTURE0 + 4);
		}

		{
			int textureLightLoc = shader->GetUniformLocation("texture_light");
			glUniform1i(textureLightLoc, 5);
			lightBuffer->BindTexture(0, GL_TEXTURE0 + 5);
		}

		// render the object again but with different properties
		RenderMesh(meshes["quad"], shader, glm::vec3(0, 0, 0));
	}
}

void Laborator6::FrameEnd()
{
	DrawCoordinatSystem();
}

void Laborator6::LoadShader(std::string name)
{
	static std::string shaderPath = "Source/Laboratoare/Laborator6/Shaders/";

	// Create a shader program for particle system
	{
		Shader *shader = new Shader(name.c_str());
		shader->AddShader((shaderPath + name + ".VS.glsl").c_str(), GL_VERTEX_SHADER);
		shader->AddShader((shaderPath + name + ".FS.glsl").c_str(), GL_FRAGMENT_SHADER);

		shader->CreateAndLink();
		shaders[shader->GetName()] = shader;
	}
}

// Read the documentation of the following functions in: "Source/Core/Window/InputController.h" or
// https://github.com/UPB-Graphics/SPG-Framework/blob/master/Source/Core/Window/InputController.h

void Laborator6::OnInputUpdate(float deltaTime, int mods)
{
	// treat continuous update based on input
};

void Laborator6::OnKeyPress(int key, int mods)
{
	// add key press event
	int index = key - GLFW_KEY_0;
	if (index >= 0 && index <= 9) {
		outputType = index;
	}
};

void Laborator6::OnKeyRelease(int key, int mods)
{
	// add key release event
};

void Laborator6::OnMouseMove(int mouseX, int mouseY, int deltaX, int deltaY)
{
	// add mouse move event
};

void Laborator6::OnMouseBtnPress(int mouseX, int mouseY, int button, int mods)
{
	// add mouse button press event
};

void Laborator6::OnMouseBtnRelease(int mouseX, int mouseY, int button, int mods)
{
	// add mouse button release event
}

void Laborator6::OnMouseScroll(int mouseX, int mouseY, int offsetX, int offsetY)
{
	// treat mouse scroll event
}

void Laborator6::OnWindowResize(int width, int height)
{
	// treat window resize event
	frameBuffer->Resize(width, height, 32);
	lightBuffer->Resize(width, height, 32);
}
