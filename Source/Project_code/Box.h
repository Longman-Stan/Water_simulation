#include <vector>
#include <include/glm.h>
#include <Core/GPU/Mesh.h>
#include <math.h>
#include <Component/SimpleScene.h>
//#include <Core/GPU/ParticleEffect.h>

struct Particl
{
	glm::vec4 position;
	glm::vec4 speed;
	glm::vec4 initialPos;
	glm::vec4 initialSpeed;

	Particl() {};

	Particl(const glm::vec4& pos, const glm::vec4& speed)
	{
		SetInitial(pos, speed);
	}

	void SetInitial(const glm::vec4& pos, const glm::vec4& speed)
	{
		position = pos;
		initialPos = pos;

		this->speed = speed;
		initialSpeed = speed;
	}
};

struct ParticleSystem
{
	ParticleEffect<Particl>* Effect;
	Particl* data;
	SSBO<Particl>* mySSBO;
	unsigned int num_particles;
};

class Box {

public:
	glm::vec3 position;
	glm::vec3 velocity;
	bool animate;
	bool colission_detected;
	glm::vec3 getPosition();
	Box();
	void setPosition(glm::vec3 pos);
	bool dead;
	float float_time;
	bool checkCollision(Box* box);
	ParticleSystem particle_system;
	void initializeParticleSystem(int num_particles);
};