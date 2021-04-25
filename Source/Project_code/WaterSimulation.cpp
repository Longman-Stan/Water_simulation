#include "WaterSimulation.h"

#include <vector>
#include <iostream>
#include <stb/stb_image.h>
#include <Core/Engine.h>
#include <math.h>
#include <Tema1/SkySphere.h>

using namespace std;

static float angle = 0;

// Order of function calling can be seen in "Source/Core/World.cpp::LoopUpdate()"
// https://github.com/UPB-Graphics/SPG-Framework/blob/master/Source/Core/World.cpp

WaterSimulation::WaterSimulation()
{
	global_time = 0;
	cubemap_side_length = 20;
	waterBody = WaterBody(1,1,0.01,20,20);

	num_gerstner_base_waves = 6;
	Q.push_back(0.9);  Q.push_back(0.7); Q.push_back(0.5);
	L.push_back(3);  L.push_back(6); L.push_back(2.5);
	A.push_back(0.06);  A.push_back(0.05); A.push_back(0.04);
	D.push_back(glm::vec3(0.5, 0 , 0.8660254)); //60 degrees
	D.push_back(glm::vec3(0.17364818, 0, 0.98480775)); //80 degrees
	D.push_back(glm::vec3(0, 0, 1)); //90 degrees
	dir.push_back(60); dir.push_back(80); dir.push_back(90);
	S.push_back(0.5);  S.push_back(0.25); S.push_back(0.75);

	Q.push_back(0.6);  Q.push_back(0.2); Q.push_back(0.5);
	L.push_back(2);  L.push_back(4); L.push_back(3);
	A.push_back(0.04);  A.push_back(0.06); A.push_back(0.05);
	D.push_back(glm::vec3(0.25881905, 0, 0.96592583)); //75 degrees
	D.push_back(glm::vec3(0.08715574, 0, 0.9961947)); //85 degrees
	D.push_back(glm::vec3(-0.17364818, 0, 0.98480775)); //100 degrees
	dir.push_back(75); dir.push_back(85); dir.push_back(100);
	S.push_back(0.2);  S.push_back(0.5); S.push_back(0.4);
	for (int i = 0; i < num_gerstner_base_waves; i++)
	{
		w.push_back( sqrt( g * 2 * M_PI / L[i]) );
		steepness.push_back(Q[i] / (w[i] * A[i] * num_gerstner_base_waves));
		phi.push_back(S[i] * w[i]);
		waves.push_back(std::string("Wave") + std::to_string(i));
	}

	num_circular_waves = 0;
	collision_center.clear();
	initial_times.clear();

	// set floating time for boxes
	box_float_time = 4;
	box_limit = 10;

	Qc = 0.3;
	Lc = 0.1;
	Ac = 0.04;
	Sc = 0.3;
	wc = sqrt(9.81 * 2 * M_PI / Lc);
	phic = Sc * wc;
	steepc = Qc / (wc * Ac);
	circular_waves_attenuation = 0.5;
	num_splash_particles = 200;

	gui_hovered = false;
}

WaterSimulation::~WaterSimulation()
{
}

void WaterSimulation::Init()
{
	auto camera = GetSceneCamera();
	camera->SetPositionAndRotation(glm::vec3(0, 5, 4), glm::quat(glm::vec3(-30 * TO_RADIANS, 0, 0)));
	camera->Update();

	std::string shaderPath = "Source/Project_code/Shaders/";

	{
		Mesh* mesh = new Mesh("cube");
		mesh->LoadMesh(RESOURCE_PATH::MODELS + "Primitives", "box.obj");
		mesh->UseMaterials(false);
		meshes[mesh->GetMeshID()] = mesh;
	}

	{
		Shader* shader = new Shader("CubeMap");
		shader->AddShader(shaderPath + "Cubeshader_VS.glsl", GL_VERTEX_SHADER);
		shader->AddShader(shaderPath + "Cubeshader_FS.glsl", GL_FRAGMENT_SHADER);
		shader->CreateAndLink();
		shaders[shader->GetName()] = shader;
	}

	{
		Shader* shader = new Shader("Box");
		shader->AddShader(shaderPath + "Box_VS.glsl", GL_VERTEX_SHADER);
		shader->AddShader(shaderPath + "Box_FS.glsl", GL_FRAGMENT_SHADER);
		shader->CreateAndLink();
		shaders[shader->GetName()] = shader;
	}

	// Create a shader program for rendering to texture
	{
		Shader* shader = new Shader("ShaderProject");
		shader->AddShader(shaderPath + "VertexShader.glsl", GL_VERTEX_SHADER);
		shader->AddShader(shaderPath + "FragmentShader.glsl", GL_FRAGMENT_SHADER);
		shader->CreateAndLink();
		shaders[shader->GetName()] = shader;
	}

	{
		Shader* shader = new Shader("Particle");
		shader->AddShader(shaderPath + "Particle_VS.glsl", GL_VERTEX_SHADER);
		shader->AddShader(shaderPath + "Particle_GS.glsl", GL_GEOMETRY_SHADER);
		shader->AddShader(shaderPath + "Particle_FS.glsl", GL_FRAGMENT_SHADER);
		shader->CreateAndLink();
		shaders[shader->GetName()] = shader;
	}

	auto resolution = window->GetResolution();

	std::string cubemap_path = "Source/Project_code/skybox/";

	cubeMapTextureID = UploadCubeMapTexture(
		cubemap_path + "right.jpg",
		cubemap_path + "top.jpg",
		cubemap_path + "front.jpg",
		cubemap_path + "left.jpg",
		cubemap_path + "bottom.jpg",
		cubemap_path + "back.jpg"
	);

	std::string texture_path = "Source/Project_code/Textures/";

	{
		Texture2D* texture = new Texture2D();
		texture->Load2D((texture_path + "water-normal.jpg").c_str(), GL_REPEAT);
		mapTextures["water-normal"] = texture;
	}

	{
		Texture2D* texture = new Texture2D();
		texture->Load2D((texture_path + "water-normal2.jpg").c_str(), GL_REPEAT);
		mapTextures["water-normal2"] = texture;
	}
	
	{
		Texture2D* texture = new Texture2D();
		texture->Load2D((texture_path + "box.jpg").c_str(), GL_REPEAT);
		mapTextures["box_texture"] = texture;
	}

	{
		Texture2D* texture = new Texture2D();
		texture->Load2D((texture_path + "water_particle.png").c_str(), GL_REPEAT);
		mapTextures["water_particle"] = texture;
	}


	lightPosition = glm::vec3(0, 20, 0);
	lightDirection = glm::vec3(0, 0, 0);
	materialShininess = 30;
	materialKd = 0.6f;
	materialKs = 0.6f;

	//gui part
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();

	ImGui::StyleColorsDark();

	const char* glsl_version = "#version 130";
	ImGui_ImplGlfw_InitForOpenGL(window->window, true);
	ImGui_ImplOpenGL3_Init(glsl_version);

	param1 = 0.0f;
	param2 = 0.0f;

	current_wave = waves[0];
	crt_wave_idx = 0;

	//enable blending
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}


unsigned int WaterSimulation::UploadCubeMapTexture(const std::string& posx, const std::string& posy, const std::string& posz, const std::string& negx, const std::string& negy, const std::string& negz)
{
	int width, height, chn;

	unsigned char* data_posx = stbi_load(posx.c_str(), &width, &height, &chn, 0);
	unsigned char* data_posy = stbi_load(posy.c_str(), &width, &height, &chn, 0);
	unsigned char* data_posz = stbi_load(posz.c_str(), &width, &height, &chn, 0);
	unsigned char* data_negx = stbi_load(negx.c_str(), &width, &height, &chn, 0);
	unsigned char* data_negy = stbi_load(negy.c_str(), &width, &height, &chn, 0);
	unsigned char* data_negz = stbi_load(negz.c_str(), &width, &height, &chn, 0);

	// TODO - create OpenGL texture
	unsigned int textureID = 0;
	glGenTextures(1, &textureID);


	// TODO - bind the texture
	glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

	glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	float maxAnisotropy;
	glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &maxAnisotropy);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, maxAnisotropy);

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	// TODO - load texture information for each face
	glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data_posx);
	glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Y, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data_posy);
	glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Z, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data_posz);
	glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_X, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data_negx);
	glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data_negy);
	glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data_negz);

	glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

	// free memory
	SAFE_FREE(data_posx);
	SAFE_FREE(data_posy);
	SAFE_FREE(data_posz);
	SAFE_FREE(data_negx);
	SAFE_FREE(data_negy);
	SAFE_FREE(data_negz);

	return textureID;
}

void WaterSimulation::FrameStart()
{

}

void WaterSimulation::Update(float deltaTimeSeconds)
{
	global_time += deltaTimeSeconds;
	ClearScreen();
	DrawCubemap();
	DrawBoxes(deltaTimeSeconds);
	DrawScene();
	DrawWaterParticles(deltaTimeSeconds);
	DrawGui();
}

void WaterSimulation::DrawBoxes(float deltaTimeSeconds)
{
	if (crtBox != nullptr)
	{
		crtBox->setPosition(GetCurrentBoxPosition());
	}
	for(vector<Box*>::iterator it = boxes.begin(); it!= boxes.end(); it++)
	{
		Box* box = *it;
		if (box->animate)
		{	
			box->position += box->velocity * deltaTimeSeconds;
			if(!box->colission_detected)
				box->velocity += glm::vec3(0, -9.81 * deltaTimeSeconds, 0);
			else
			{
				if (box->float_time < 0)
					box->velocity += glm::vec3(0, -0.4 * deltaTimeSeconds, 0);
				box->float_time -= deltaTimeSeconds;
			}

			if (box->position[1] < -3)
			{
				box->animate = 0;
				box->dead = true;
				continue;
			}

			if( box->position[1] < 0 && !box->colission_detected)
			{
				box->colission_detected = 1;
				num_circular_waves++;
				initial_times.push_back(global_time - 0.0001);
				glm::vec3 col_pos = box->position;
				col_pos.y = 0;
				collision_center.push_back(col_pos);
				box->velocity = glm::vec3(0, 0, 0);
				box->float_time = box_float_time;
				box->initializeParticleSystem(num_splash_particles);
			}
		}
		DrawCube(box);
	}

	for (int i = boxes.size()-1; i >= 0; i--)
	{
		if (boxes[i]->dead)
			continue;
		for (int j = i - 1; j >= 0; j--)
		{
			if (boxes[j]->dead)
				continue;
			if (boxes[i]->checkCollision(boxes[j]))
			{
				boxes[j]->float_time = -0.01f;
			}
		}
	}
	for (int i = boxes.size() - 1; i >= 0; i--)
	{
		if (boxes[i]->dead)
		{
			Box* b = boxes[i];
			boxes.erase(boxes.begin() + i);
			delete b;
		}
	}
}

void WaterSimulation::DrawGui()
{
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();

	ImGui::Begin("Custom parameters");

	if (ImGui::BeginCombo("Select Wave", current_wave.c_str()))
	{
		for (int i = 0; i < num_gerstner_base_waves; i++)
		{
			bool is_selected = (current_wave == waves[i]);

			if (ImGui::Selectable(waves[i].c_str(), is_selected))
			{
				crt_wave_idx = i;
				current_wave = waves[i];
			}

			if (is_selected)
				ImGui::SetItemDefaultFocus();
		}

		ImGui::EndCombo();
	}
	gui_hovered = ImGui::IsAnyWindowHovered() || ImGui::IsAnyItemHovered() || ImGui::IsAnyItemFocused();

	if (crt_wave_idx != -1)
	{
		ImGui::InputFloat("Amplitude(A)", &A[crt_wave_idx]);
		ImGui::InputFloat("Wavelength(L)", &L[crt_wave_idx]);
		ImGui::InputFloat("Speed(S)", &S[crt_wave_idx]);
		ImGui::InputFloat("Direction Angle(D)", &dir[crt_wave_idx]);
		ImGui::InputFloat("Wave steepness(Q)", &Q[crt_wave_idx]);

		float dval = dir[crt_wave_idx];
		D[crt_wave_idx] = glm::vec3(cos(dval * TO_RADIANS), 0, sin(dval * TO_RADIANS));
		w[crt_wave_idx] = sqrt(g * 2 * M_PI / L[crt_wave_idx]);
		steepness[crt_wave_idx] = Q[crt_wave_idx] / (w[crt_wave_idx] * A[crt_wave_idx] * num_gerstner_base_waves);
		phi[crt_wave_idx] = S[crt_wave_idx] * w[crt_wave_idx];
	}
	if (ImGui::Button("Add"))
	{
		num_gerstner_base_waves++;
		Q.push_back(0.0f);
		L.push_back(1.0f);
		A.push_back(0.001f);
		D.push_back(glm::vec3(0));
		dir.push_back(0);
		S.push_back(0.0f);
		w.push_back(0);
		steepness.push_back(0);
		phi.push_back(0);
		waves.push_back(std::string("Wave") + std::to_string(waves.size()));
		current_wave = waves.back();
		crt_wave_idx = waves.size() - 1;
	}

	ImGui::SameLine();

	if (ImGui::Button("Remove"))
	{
		num_gerstner_base_waves--;
		Q.erase(Q.begin() + crt_wave_idx);
		L.erase(L.begin() + crt_wave_idx);
		A.erase(A.begin() + crt_wave_idx);
		D.erase(D.begin() + crt_wave_idx);
		dir.erase(dir.begin() + crt_wave_idx);
		S.erase(S.begin() + crt_wave_idx);
		w.erase(w.begin() + crt_wave_idx);
		steepness.erase(steepness.begin() + crt_wave_idx);
		phi.erase(phi.begin() + crt_wave_idx);
		if (crt_wave_idx == waves.size() - 1)
		{
			crt_wave_idx--;
			current_wave = waves.back();
		}
		waves.pop_back();
	}

	ImGui::End();

	ImGui::Render();

	int display_w, display_h;
	glfwGetFramebufferSize(window->window, &display_w, &display_h);
	glViewport(0, 0, display_w, display_h);

	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void WaterSimulation::DrawCube(Box* box)
{

	Shader* shader = shaders["Box"];
	shader->Use();

	Camera* camera = GetSceneCamera();

	{

		int light_position = glGetUniformLocation(shader->program, "light_position");
		glUniform3f(light_position, lightPosition.x, lightPosition.y, lightPosition.z);

		int light_direction = glGetUniformLocation(shader->program, "light_direction");
		glUniform3f(light_direction, lightDirection.x, lightDirection.y, lightDirection.z);

		int loc_eyepos = glGetUniformLocation(shader->program, "eye_position");
		glUniform3fv(loc_eyepos, 1, glm::value_ptr(camera->transform->GetWorldPosition()));

		int material_shininess = glGetUniformLocation(shader->program, "material_shininess");
		glUniform1i(material_shininess, materialShininess);

		int material_kd = glGetUniformLocation(shader->program, "material_kd");
		glUniform1f(material_kd, materialKd);

		int material_ks = glGetUniformLocation(shader->program, "material_ks");
		glUniform1f(material_ks, materialKs);
	}

	{
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, mapTextures["box_texture"]->GetTextureID());
		int loc_texture = shader->GetUniformLocation("box_texture");
		glUniform1i(loc_texture, 0);
	}

	{
		glm::mat4 modelMatrix = glm::mat4(1);
		modelMatrix = glm::translate(modelMatrix, GetBoxGernstPosition(box));
		modelMatrix = glm::scale(modelMatrix, glm::vec3(0.2f));

		glUniformMatrix4fv(shader->loc_model_matrix, 1, GL_FALSE, glm::value_ptr(modelMatrix));
		glUniformMatrix4fv(shader->loc_view_matrix, 1, GL_FALSE, glm::value_ptr(camera->GetViewMatrix()));
		glUniformMatrix4fv(shader->loc_projection_matrix, 1, GL_FALSE, glm::value_ptr(camera->GetProjectionMatrix()));
	}

	meshes["cube"]->Render();
	//RenderMesh(meshes["cube"], shader, glm::vec3(0.f, 3.f, 0.f), glm::vec3(0.2f));
}

glm::vec3 WaterSimulation::GetBoxGernstPosition(Box* box)
{
	glm::vec3 new_vec = box->position;
	float time = global_time;

	for (int i = 0; i < num_gerstner_base_waves; i++)
	{
		float gernst = w[i] * glm::dot(D[i], box->position) + time * phi[i];
		float val_cos = cos(gernst);
		new_vec.x += steepness[i] * A[i] * D[i].x * val_cos;
		new_vec.z += steepness[i] * A[i] * D[i].z * val_cos;
		new_vec.y += A[i] * sin(gernst);
	}

	for (int i = 0; i < num_circular_waves; i++)
	{
		glm::vec3 Dc = glm::vec3(0, 0, 0);
		float len = 0;
		glm::vec3 col_dir = box->position - collision_center[i];

		if (box->position != collision_center[i])
		{
			len = length(col_dir);
			Dc = -normalize(col_dir);
		}

		float gernst = wc * dot(Dc, col_dir) + time * phic;
		float val_cos = cos(gernst);
		float attenuation_coeff = exp(0.1f * (initial_times[i] - time));

		new_vec.x += steepc * Ac * Dc.x * val_cos * attenuation_coeff * circular_waves_attenuation;
		new_vec.z += steepc * Ac * Dc.z * val_cos * attenuation_coeff * circular_waves_attenuation;
		new_vec.y += Ac * sin(gernst) * 2 * exp(-2.f * len) * attenuation_coeff * circular_waves_attenuation;
	}
	return new_vec;
}

void WaterSimulation::DrawCubemap()
{
	auto camera = GetSceneCamera();

	Shader* shader = shaders["CubeMap"];
	shader->Use();

	glm::mat4 modelMatrix = glm::scale(glm::mat4(1), glm::vec3(cubemap_side_length));

	glUniformMatrix4fv(shader->loc_model_matrix, 1, GL_FALSE, glm::value_ptr(modelMatrix));
	glUniformMatrix4fv(shader->loc_view_matrix, 1, GL_FALSE, glm::value_ptr(camera->GetViewMatrix()));
	glUniformMatrix4fv(shader->loc_projection_matrix, 1, GL_FALSE, glm::value_ptr(camera->GetProjectionMatrix()));

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, cubeMapTextureID);
	int loc_texture = shader->GetUniformLocation("texture_cubemap");
	glUniform1i(loc_texture, 0);

	meshes["cube"]->Render();
}

void WaterSimulation::DrawScene()
{
	//glLineWidth(1);
	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	Shader* shader = shaders["ShaderProject"];
	shader->Use();

	{
		Camera *camera = GetSceneCamera();

		int light_position = glGetUniformLocation(shader->program, "light_position");
		glUniform3f(light_position, lightPosition.x, lightPosition.y, lightPosition.z);

		int light_direction = glGetUniformLocation(shader->program, "light_direction");
		glUniform3f(light_direction, lightDirection.x, lightDirection.y, lightDirection.z);

		int loc_eyepos = glGetUniformLocation(shader->program, "eye_position");
		glUniform3fv(loc_eyepos, 1, glm::value_ptr(camera->transform->GetWorldPosition()));

		int material_shininess = glGetUniformLocation(shader->program, "material_shininess");
		glUniform1i(material_shininess, materialShininess);

		int material_kd = glGetUniformLocation(shader->program, "material_kd");
		glUniform1f(material_kd, materialKd);

		int material_ks = glGetUniformLocation(shader->program, "material_ks");
		glUniform1f(material_ks, materialKs);

	}

	//send uniforms
	{
		int loc_time = glGetUniformLocation(shader->program, "time");
		glUniform1f(loc_time, global_time);

		int loc_num_gernst_waves = glGetUniformLocation(shader->program, "num_g_waves");
		glUniform1i(loc_num_gernst_waves, num_gerstner_base_waves);

		if (num_gerstner_base_waves)
		{
			int loc_Q = glGetUniformLocation(shader->program, "Q");
			glUniform1fv(loc_Q, num_gerstner_base_waves, Q.data());

			int loc_A = glGetUniformLocation(shader->program, "A");
			glUniform1fv(loc_A, num_gerstner_base_waves, A.data());

			int loc_D = glGetUniformLocation(shader->program, "D");
			glUniform3fv(loc_D, num_gerstner_base_waves, glm::value_ptr(D[0]));

			int loc_w = glGetUniformLocation(shader->program, "w");
			glUniform1fv(loc_w, num_gerstner_base_waves, w.data());

			int loc_phi = glGetUniformLocation(shader->program, "phi");
			glUniform1fv(loc_phi, num_gerstner_base_waves, phi.data());

			int loc_steep = glGetUniformLocation(shader->program, "steepness");
			glUniform1fv(loc_steep, num_gerstner_base_waves, steepness.data());

		}

		{
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_CUBE_MAP, cubeMapTextureID);
			int loc_texture = shader->GetUniformLocation("texture_cubemap");
			glUniform1i(loc_texture, 0);
		}

		{
			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D, mapTextures["water-normal"]->GetTextureID());
			int loc_texture = shader->GetUniformLocation("texture_water");
			glUniform1i(loc_texture, 1);
		}
		
		int loc_num_circ_waves = glGetUniformLocation(shader->program, "num_c_waves");
		glUniform1i(loc_num_circ_waves, num_circular_waves);

		if (num_circular_waves)
		{
			int loc_times = glGetUniformLocation(shader->program, "initial_times");
			glUniform1fv(loc_times, num_circular_waves, initial_times.data());

			int loc_col_center = glGetUniformLocation(shader->program, "collision_centers");
			glUniform3fv(loc_col_center, num_circular_waves, glm::value_ptr(collision_center[0]));
		}
	}

	for (auto patch : waterBody.patches)
	{
		RenderMesh(patch.getMesh(), shader, glm::mat4(1));
	}


	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

}

void WaterSimulation::FrameEnd()
{
//	DrawCoordinatSystem();
}

void WaterSimulation::DrawWaterParticles(float dt)
{
	auto shader = shaders["Particle"];
	auto camera = GetSceneCamera();
	if (shader->GetProgramID())
	{
		shader->Use();
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, mapTextures["water_particle"]->GetTextureID());
		int loc_texture = shader->GetUniformLocation("water_particle");
		glUniform1i(loc_texture, 1);

		glUniform1f(shader->GetUniformLocation("global_time"), global_time);
		glUniform1f(shader->GetUniformLocation("dt"), dt);

		for (auto box : boxes)
		{
			if(box->colission_detected)
				box->particle_system.Effect->Render(camera, shader);
		}
	}
}

// Read the documentation of the following functions in: "Source/Core/Window/InputController.h" or
// https://github.com/UPB-Graphics/SPG-Framework/blob/master/Source/Core/Window/InputController.h

void WaterSimulation::OnInputUpdate(float deltaTime, int mods)
{
	// treat continuous update based on input
};

void WaterSimulation::OnKeyPress(int key, int mods)
{
	// add key press event
	if (key == GLFW_KEY_SPACE)
	{
		//animate_cube = 1 - animate_cube;
		//colission_detected = 0;
		//cube_positions[0][1] = 15;
		//cube_velocity[0][1] = 0;
	}
};

glm::vec3 WaterSimulation::GetCurrentBoxPosition()
{
	Camera* camera = GetSceneCamera();
	glm::mat4 view = camera->GetViewMatrix();

	glm::mat4 view_inv = glm::inverse(view);
	glm::vec4 forward = view_inv * glm::vec4(0, 0, 1, 0);

	float px, py, pz;
	px = view_inv[3][0];
	py = view_inv[3][1];
	pz = view_inv[3][2];
	glm::vec3 new_pos = glm::vec3(0);
	if ( forward.y == 0)
		return new_pos;

	float t = py / forward.y;
	new_pos[0] = px + t * -forward.x;
	new_pos[1] = min(10, 0.5 * py);
	new_pos[2] = pz + t * - forward.z;
	return new_pos;
}

void WaterSimulation::OnKeyRelease(int key, int mods)
{
	// add key release event
};

void WaterSimulation::OnMouseMove(int mouseX, int mouseY, int deltaX, int deltaY)
{
	// add mouse move event
};

void WaterSimulation::OnMouseBtnPress(int mouseX, int mouseY, int button, int mods)
{
	// add mouse button press event

	//cube_positions.push_back(glm::vec3(0, 15, 0));
	//cube_velocity.push_back(glm::vec3(0, 0, 0));
	//animate_cube = 0;
	//colission_detected = 0;
	if (button == 1 && boxes.size() < box_limit && !gui_hovered)
	{
		crtBox = new Box();
		crtBox->setPosition(GetCurrentBoxPosition());
		boxes.push_back(crtBox);
	}
};

void WaterSimulation::OnMouseBtnRelease(int mouseX, int mouseY, int button, int mods)
{
	// add mouse button release event
	if (button == 1 && crtBox != nullptr)
	{
		crtBox->animate = true;
		crtBox = nullptr;
	}
}

void WaterSimulation::OnMouseScroll(int mouseX, int mouseY, int offsetX, int offsetY)
{
	// treat mouse scroll event
}

void WaterSimulation::OnWindowResize(int width, int height)
{
	// treat window resize event
}
