#include "Box.h"

Box::Box()
{
	position = glm::vec3(0);
	velocity = glm::vec3(0);
	animate = colission_detected = dead = false;
	float_time = 0;
}

void Box::setPosition(glm::vec3 pos)
{
	position = pos;
}

glm::vec3 Box::getPosition()
{
	return position;
}

bool Box::checkCollision(Box* box)
{
	glm::vec3 ul, ur, ll, lr;
	glm::vec3 bul, bur, bll, blr;
	ul = position + glm::vec3(-0.1f, -0.1f, -0.1f);
	ur = position + glm::vec3(0.1f, -0.1f, -0.1f);
	ll = position + glm::vec3(-0.1f, -0.1f, 0.1f);
	lr = position + glm::vec3(0.1f, -0.1f, 0.1f);

	bul = box->position + glm::vec3(-0.1f, 0.1f, -0.1f);
	bur = box->position + glm::vec3(0.1f, 0.1f, -0.1f);
	bll = box->position + glm::vec3(-0.1f, 0.1f, 0.1f);
	blr = box->position + glm::vec3(0.1f, 0.1f, 0.1f);

	if (ul.y > bul.y) //all have the same y
		return false; //if your bottom is higher than the highest point of the other box, you're not touching it

	if (ul.x >= bul.x && ul.x <= bur.x && ul.z >= bul.z && ul.z <= bll.z)
		return true;
	if (ur.x >= bul.x && ur.x <= bur.x && ur.z >= bul.z && ur.z <= bll.z)
		return true;
	if (ll.x >= bul.x && ll.x <= bur.x && ll.z >= bul.z && ll.z <= bll.z)
		return true;
	if (lr.x >= bul.x && lr.x <= bur.x && lr.z >= bul.z && lr.z <= bll.z)
		return true;

	return false;
}

void Box::initializeParticleSystem(int num_particles)
{
	particle_system.num_particles = num_particles;
	{
		particle_system.Effect = new ParticleEffect<Particl>();
		particle_system.Effect->Generate(num_particles, true);
		particle_system.mySSBO = particle_system.Effect->GetParticleBuffer();
		particle_system.data = const_cast<Particl*>(particle_system.mySSBO->GetBuffer());
	}

	for (unsigned int i = 0; i < num_particles; i++)
	{
		glm::vec4 pos(0);
		glm::vec4 speed(0);

		pos.x = (float)(rand() % 200 - 100) / 1000.f;
		pos.y = (float)(rand() % 1000) / 1000.f;
		pos.z = (float)(rand() % 200 - 100) / 1000.f;
		pos += glm::vec4(position,0);

		speed.x = (float)(rand() % 2000 - 1000) / 3000.f;
		speed.y = (float)(rand() % 1000) / 2000.f;
		speed.z = (float)(rand() % 2000 - 1000) / 3000.f;
		particle_system.data[i].SetInitial(pos, speed);
	}
	particle_system.mySSBO->SetBufferData(particle_system.data);
}