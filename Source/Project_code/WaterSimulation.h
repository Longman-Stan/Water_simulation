#pragma once
#include "WaterBody.h"
#include "Box.h"
#include <include/imgui/imgui.h>
#include <include/imgui/imgui_impl_glfw.h>
#include <include/imgui/imgui_impl_opengl3.h>

#include <Component/SimpleScene.h>
#include <vector>

#define g 9.81

class WaterSimulation : public SimpleScene
{
public:
	WaterSimulation();
	~WaterSimulation();

	void Init() override;

private:
	void FrameStart() override;
	void Update(float deltaTimeSeconds) override;
	void FrameEnd() override;

	void DrawScene();

	void OnInputUpdate(float deltaTime, int mods) override;
	void OnKeyPress(int key, int mods) override;
	void OnKeyRelease(int key, int mods) override;
	void OnMouseMove(int mouseX, int mouseY, int deltaX, int deltaY) override;
	void OnMouseBtnPress(int mouseX, int mouseY, int button, int mods) override;
	void OnMouseBtnRelease(int mouseX, int mouseY, int button, int mods) override;
	void OnMouseScroll(int mouseX, int mouseY, int offsetX, int offsetY) override;
	void OnWindowResize(int width, int height) override;

	unsigned int UploadCubeMapTexture(const std::string& posx, const std::string& posy, const std::string& posz, const std::string& negx, const std::string& negy, const std::string& negz);
	void DrawCubemap();
	void DrawBoxes(float deltaTimeSeconds);
	void DrawCube(Box* box);

	//define cubemap sizes
	float cubemap_side_length;

	WaterBody waterBody;
	float global_time;

	int cubeMapTextureID;

	//declare the variables for the collision waves
	int num_circular_waves;
	std::vector<float> initial_times;
	std::vector<glm::vec3> collision_center;

	//declare the variables for the gernster waves
	int num_gerstner_base_waves;
	std::vector<float> Q;
	std::vector<float> L;
	std::vector<float> A;
	std::vector<float> dir;
	std::vector<glm::vec3> D;
	std::vector<float> S;
	std::vector<float> steepness;
	std::vector<float> w;
	std::vector<float> phi;

	glm::vec3 lightPosition;
	glm::vec3 lightDirection;
	unsigned int materialShininess;
	float materialKd;
	float materialKs;

	std::unordered_map<std::string, Texture2D*> mapTextures;

	//Boxes
	float box_float_time;
	float box_limit;
	glm::vec3 GetCurrentBoxPosition();
	Box* crtBox;
	std::vector<Box*> boxes;
	glm::vec3 GetBoxGernstPosition(Box* box);

	float Qc = 0.3;
	float Lc = 0.1;
	float Ac = 0.04;
	float Sc = 0.3;
	float wc = sqrt(9.81 * 2 * M_PI / Lc);
	float phic = Sc * wc;
	float steepc = Qc / (wc * Ac);
	float circular_waves_attenuation;
	void DrawWaterParticles(float dt);
	int num_splash_particles;

	// paranama
	void DrawGui();
	float param1, param2;
	std::vector<std::string> waves;
	std::string current_wave;
	bool gui_hovered;
	int crt_wave_idx;
};
