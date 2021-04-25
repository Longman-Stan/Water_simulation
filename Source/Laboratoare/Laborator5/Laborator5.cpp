#include "Laborator5.h"

#include <vector>
#include <iostream>

#include <Core/Engine.h>


using namespace std;

struct Particle
{
	glm::vec4 position;
	glm::vec4 speed;
	glm::vec4 initialPos;
	glm::vec4 initialSpeed;

	Particle() {};

	Particle(const glm::vec4 &pos, const glm::vec4 &speed)
	{
		SetInitial(pos, speed);
	}

	void SetInitial(const glm::vec4 &pos, const glm::vec4 &speed)
	{
		position = pos;
		initialPos = pos;

		this->speed = speed;
		initialSpeed = speed;
	}
};

ParticleEffect<Particle> *particleEffect;

// Order of function calling can be seen in "Source/Core/World.cpp::LoopUpdate()"
// https://github.com/UPB-Graphics/SPG-Framework/blob/master/Source/Core/World.cpp

Laborator5::Laborator5()
{
}

Laborator5::~Laborator5()
{
}

void Laborator5::Init()
{
	auto camera = GetSceneCamera();
	camera->SetPositionAndRotation(glm::vec3(0, 8, 8), glm::quat(glm::vec3(-40 * TO_RADIANS, 0, 0)));
	camera->Update();

	{
		Mesh* mesh = new Mesh("box");
		mesh->LoadMesh(RESOURCE_PATH::MODELS + "Primitives", "box.obj");
		meshes[mesh->GetMeshID()] = mesh;
	}

	// Load textures
	{
		TextureManager::LoadTexture(RESOURCE_PATH::TEXTURES, "particle2.png");
	}
	{
		TextureManager::LoadTexture(RESOURCE_PATH::TEXTURES, "blue.jpg");
	}

	LoadShader("Simple", false);
	LoadShader("Particle");

	unsigned int nrParticles = 50'000;

	particleEffect = new ParticleEffect<Particle>();
	particleEffect->Generate(nrParticles, true);

	auto particleSSBO = particleEffect->GetParticleBuffer();
	Particle* data = const_cast<Particle*>(particleSSBO->GetBuffer());

	int cubeSize = 20;
	int hSize = cubeSize / 2;

	for (unsigned int i = 0; i < nrParticles; i++)
	{
		glm::vec4 pos(1);
		pos.x = (rand() % cubeSize - hSize) / 10.0f;
		pos.y = (rand() % cubeSize - hSize) / 10.0f;
		pos.z = (rand() % cubeSize - hSize) / 10.0f;

		glm::vec4 speed(0);
		speed.x = (rand() % 20 - 10) / 10.0f;
		speed.z = (rand() % 20 - 10) / 10.0f;
		speed.y = rand() % 2 + 2.0f;

		data[i].SetInitial(pos, speed);
	}
	particleSSBO->SetBufferData(data);


}


void Laborator5::FrameStart()
{
	// clears the color buffer (using the previously set color) and depth buffer
	glClearColor(0, 0, 0, 1);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glm::ivec2 resolution = window->GetResolution();
	// sets the screen area where to draw
	glViewport(0, 0, resolution.x, resolution.y);
}


void Laborator5::Update(float deltaTimeSeconds)
{
	glLineWidth(3);

	glEnable(GL_BLEND);
	glDisable(GL_DEPTH_TEST);
	glBlendFunc(GL_ONE, GL_ONE);
	glBlendEquation(GL_FUNC_ADD);

	{
		auto shader = shaders["Particle"];
		if (shader->GetProgramID())
		{
			shader->Use();
			TextureManager::GetTexture("particle2.png")->BindToTextureUnit(GL_TEXTURE0);
			particleEffect->Render(GetSceneCamera(), shader);
		}
	}

	glEnable(GL_DEPTH_TEST);
	glDisable(GL_BLEND);

	{
		glm::mat4 model = glm::translate(glm::mat4(1), glm::vec3(3, 0, 0));
		RenderMesh(meshes["box"], shaders["Simple"], model);
	}
}

void Laborator5::FrameEnd()
{
	//DrawCoordinatSystem();
}

void Laborator5::LoadShader(std::string name, bool hasGeomtery)
{
	static std::string shaderPath = "Source/Laboratoare/Laborator5/Shaders/";

	// Create a shader program for particle system
	{
		Shader *shader = new Shader(name.c_str());
		shader->AddShader((shaderPath + name + ".VS.glsl").c_str(), GL_VERTEX_SHADER);
		shader->AddShader((shaderPath + name + ".FS.glsl").c_str(), GL_FRAGMENT_SHADER);
		if (hasGeomtery)
		{
			shader->AddShader((shaderPath + name + ".GS.glsl").c_str(), GL_GEOMETRY_SHADER);
		}

		shader->CreateAndLink();
		shaders[shader->GetName()] = shader;
	}
}

// Read the documentation of the following functions in: "Source/Core/Window/InputController.h" or
// https://github.com/UPB-Graphics/SPG-Framework/blob/master/Source/Core/Window/InputController.h

void Laborator5::OnInputUpdate(float deltaTime, int mods)
{
	// treat continuous update based on input
};

void Laborator5::OnKeyPress(int key, int mods)
{
};

void Laborator5::OnKeyRelease(int key, int mods)
{
	// add key release event
};

void Laborator5::OnMouseMove(int mouseX, int mouseY, int deltaX, int deltaY)
{
	// add mouse move event
};

void Laborator5::OnMouseBtnPress(int mouseX, int mouseY, int button, int mods)
{
	// add mouse button press event
};

void Laborator5::OnMouseBtnRelease(int mouseX, int mouseY, int button, int mods)
{
	// add mouse button release event
}

void Laborator5::OnMouseScroll(int mouseX, int mouseY, int offsetX, int offsetY)
{
	// treat mouse scroll event
}

void Laborator5::OnWindowResize(int width, int height)
{
	// treat window resize event
}
