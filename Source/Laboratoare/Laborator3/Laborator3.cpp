#include "Laborator3.h"

#include <vector>
#include <iostream>

#include <Core/Engine.h>

using namespace std;

static float angle = 0;

// Order of function calling can be seen in "Source/Core/World.cpp::LoopUpdate()"
// https://github.com/UPB-Graphics/SPG-Framework/blob/master/Source/Core/World.cpp

Laborator3::Laborator3()
{
}

Laborator3::~Laborator3()
{
}

void Laborator3::Init()
{
	auto camera = GetSceneCamera();
	ProjectionInfo pi = camera->GetProjectionInfo();
	printf("%f %f\n", pi.zNear, pi.zFar);
	camera->SetPositionAndRotation(glm::vec3(0, 5, 4), glm::quat(glm::vec3(-30 * TO_RADIANS, 0, 0)));
	camera->Update();

	// Load a mesh from file into GPU memory
	{
		Mesh* mesh = new Mesh("bamboo");
		mesh->LoadMesh(RESOURCE_PATH::MODELS + "Vegetation/Bamboo", "bamboo.obj");
		meshes[mesh->GetMeshID()] = mesh;
	}

	{
		Mesh* mesh = new Mesh("quad");
		mesh->LoadMesh(RESOURCE_PATH::MODELS + "Primitives", "quad.obj");
		mesh->UseMaterials(false);
		meshes[mesh->GetMeshID()] = mesh;
	}

	std::string shaderPath = "Source/Laboratoare/Laborator3/Shaders/";

	// Create a shader program for rendering to texture
	{
		Shader *shader = new Shader("ShaderLab3");
		shader->AddShader(shaderPath + "VertexShader.glsl", GL_VERTEX_SHADER);
		shader->AddShader(shaderPath + "FragmentShader.glsl", GL_FRAGMENT_SHADER);
		shader->CreateAndLink();
		shaders[shader->GetName()] = shader;

		Shader* shader2 = new Shader("ShaderLab3_1");
		shader2->AddShader(shaderPath + "VertexShader.glsl", GL_VERTEX_SHADER);
		shader2->AddShader(shaderPath + "FragmentShader2.glsl", GL_FRAGMENT_SHADER);
		shader2->CreateAndLink();
		shaders[shader2->GetName()] = shader2;

	}

	auto resolution = window->GetResolution();

	frameBuffer = new FrameBuffer();
	frameBuffer->Clean();
	frameBuffer->Clear();

	frameBuffer->Generate(resolution.x , resolution.y, 1, true);
}

void Laborator3::FrameStart()
{

}

void Laborator3::Update(float deltaTimeSeconds)
{
	ClearScreen();
	static glm::vec3 mirrorPos(0, 4, -6);
	static glm::vec3 mirrorRotation = glm::vec3(0, RADIANS(180), 0);

	angle += 0.5f * deltaTimeSeconds;

	ClearScreen();
	
	// Save camera position and rotation
	auto camera = GetSceneCamera();

	glm::vec3 camPosition = camera->transform->GetWorldPosition();
	glm::quat camRotation = camera->transform->GetWorldRotation();

	//TODO - Render scene view from the mirror point of view
	// Use camera->SetPosition() and camera->SetRotation(glm::quat(euler_angles)) 
	{
		frameBuffer->Bind();
		camera->SetPosition(mirrorPos);
		camera->SetRotation(mirrorRotation);

		DrawScene();
		frameBuffer->BindDefault();
	}

	// Render the scene normaly
	{
		camera->SetPosition(camPosition);
		camera->SetRotation(camRotation);

		DrawScene();
	}

	//TODO - Render the mirror view
	{
		auto shader = shaders["ShaderLab3_1"];
		// TODO - Use mirror texture
		//frameBuffer->BindTexture(0, GL_TEXTURE0);
		frameBuffer->BindDepthTexture(GL_TEXTURE0);
		
		glm::mat4 modelMatrix(1);
		modelMatrix = glm::translate(modelMatrix, mirrorPos);
		modelMatrix = glm::scale(modelMatrix, glm::vec3(4));
		RenderMesh(meshes["quad"], shader, modelMatrix);

	}
}

void Laborator3::DrawScene()
{

	for (int i = 0; i < 16; i++)
	{
		float rotateAngle = (angle + i) * ((i % 2) * 2 - 1);
		glm::vec3 position = glm::vec3(-4 + (i % 4) * 2.5, 0, -2 + (i / 4) * 2.5);

		glm::mat4 modelMatrix = glm::translate(glm::mat4(1), position);
		modelMatrix = glm::rotate(modelMatrix, rotateAngle, glm::vec3(0, 1, 0));
		modelMatrix = glm::scale(modelMatrix, glm::vec3(0.1f));

		RenderMesh(meshes["bamboo"], shaders["ShaderLab3"], modelMatrix);
	}
}

void Laborator3::FrameEnd()
{
	DrawCoordinatSystem();
}

// Read the documentation of the following functions in: "Source/Core/Window/InputController.h" or
// https://github.com/UPB-Graphics/SPG-Framework/blob/master/Source/Core/Window/InputController.h

void Laborator3::OnInputUpdate(float deltaTime, int mods)
{
	// treat continuous update based on input
};

void Laborator3::OnKeyPress(int key, int mods)
{
	// add key press event
};

void Laborator3::OnKeyRelease(int key, int mods)
{
	// add key release event
};

void Laborator3::OnMouseMove(int mouseX, int mouseY, int deltaX, int deltaY)
{
	// add mouse move event
};

void Laborator3::OnMouseBtnPress(int mouseX, int mouseY, int button, int mods)
{
	// add mouse button press event
};

void Laborator3::OnMouseBtnRelease(int mouseX, int mouseY, int button, int mods)
{
	// add mouse button release event
}

void Laborator3::OnMouseScroll(int mouseX, int mouseY, int offsetX, int offsetY)
{
	// treat mouse scroll event
}

void Laborator3::OnWindowResize(int width, int height)
{
	// treat window resize event
}
