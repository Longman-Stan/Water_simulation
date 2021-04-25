#version 430

layout(location = 0) in vec3 v_position;
layout(location = 1) in vec3 v_normal;
layout(location = 2) in vec2 v_texture_coord;

// Uniform properties
uniform mat4 Model;
uniform mat4 View;
uniform mat4 Projection;

uniform float time;

uniform int num_g_waves;
uniform float Q[10];
uniform float A[10];
uniform vec3 D[10];
uniform float w[10];
uniform float phi[10];
uniform float steepness[10];

//for circular waves
uniform int num_c_waves;
uniform float initial_times[50];
uniform vec3 collision_centers[50];

float Qc,Lc,Ac,Sc,wc,phic,steepc;

layout(location = 0) out vec2 texture_coord;
layout(location = 1) out vec3 T;
layout(location = 2) out vec3 N;
layout(location = 3) out vec3 B;
layout(location = 4) out vec3 world_position;
layout(location = 5) out vec3 world_normal;

#define M_PI 3.1415926535897932384626433832795
#define EXP_LEN 2
#define EXP_TIME 0.1

vec3 gernster_position(vec3 position)
{
	vec3 new_vec = position;
	for(int i = 0; i < num_g_waves; i++)
	{
		float gernst = w[i] * dot( D[i], v_position) + time * phi[i];
		float val_cos = cos(gernst);
		new_vec.x += steepness[i] * A[i] * D[i].x * val_cos;
		new_vec.z += steepness[i] * A[i] * D[i].z * val_cos;
		new_vec.y += A[i] * sin(gernst);
	}

	for(int i = 0; i < num_c_waves; i++)
	{
		vec3 Dc = vec3(0,0,0);
		float len = 0;
		vec3 col_dir = position - collision_centers[i];

		if( position != collision_centers[i] )
		{
			len = length(col_dir);
			Dc = -normalize(col_dir);
		}

		float gernst =  wc * dot(Dc , col_dir) + time * phic;
		float val_cos = cos(gernst);
		float attenuation_coeff = exp(EXP_TIME*(initial_times[i] - time));

		new_vec.x += steepc * Ac * Dc.x * val_cos * attenuation_coeff;
		new_vec.z += steepc * Ac * Dc.z * val_cos * attenuation_coeff;
		new_vec.y += Ac * sin(gernst) * 2 * exp(-EXP_LEN*len) * attenuation_coeff;
	}

	return new_vec;
}

vec3 get_B(vec3 position)
{
	vec3 rez = vec3(1,0,0);
	for(int i = 0; i < num_g_waves; i++)
	{
		float wa = w[i] * A[i];
		float gernst = w[i] * dot( D[i] , position) + time * phi[i];
		float s = sin(gernst);
		float c = cos(gernst);
		rez.x -= Q[i] * D[i].x * D[i].x * wa * s;
		rez.z -= Q[i] * D[i].x * D[i].z * wa * s;
		rez.y += D[i].x * wa * c;
	}
		for(int i = 0; i < num_c_waves; i++)
	{
		vec3 Dc = vec3(0,0,0);
		float len = 0;
		vec3 col_dir = position - collision_centers[i];

		if( position != collision_centers[i] )
		{
			len = length(col_dir);
			Dc = -normalize(col_dir);
		}
		//len *= len*len;
		//if(len < 1) len = 1;

		float wa = wc * Ac;
		float gernst =  wc * dot(Dc , col_dir) + time * phic;
		float s = sin(gernst);
		float c = cos(gernst);

		float attenuation_coeff = exp(EXP_TIME*(initial_times[i] - time));
		rez.x -= Qc * Dc.x * Dc.x * wa * s * attenuation_coeff * exp(-EXP_LEN*len);
		rez.z -= Qc * Dc.x * Dc.z * wa * s * attenuation_coeff * exp(-EXP_LEN*len);
		rez.y += Dc.x * wa * c * attenuation_coeff * exp(-EXP_LEN*len);
	}
	return rez;
}

vec3 get_T(vec3 position)
{
	vec3 rez = vec3(0,0,1);
	for(int i = 0; i < num_g_waves; i++)
	{
		float wa = w[i] * A[i];
		float gernst = w[i] * dot( D[i] , position) + time * phi[i];
		float s = sin(gernst);
		float c = cos(gernst);
		rez.x -= Q[i] * D[i].x * D[i].z * wa * s;
		rez.z -= Q[i] * D[i].z * D[i].z * wa * s;
		rez.y += D[i].z * wa * c;
	}
	for(int i = 0; i < num_c_waves; i++)
	{
		vec3 Dc = vec3(0,0,0);
		float len = 0;
		vec3 col_dir = position - collision_centers[i];

		if( position != collision_centers[i] )
		{
			len = length(col_dir);
			Dc = -normalize(col_dir);
		}
		//len *= len*len;
		//if(len < 1) len = 1;


		float wa = wc * Ac;
		float gernst =  wc * dot(Dc , col_dir) + time * phic;
		float s = sin(gernst);
		float c = cos(gernst);

		float attenuation_coeff = exp(EXP_TIME*(initial_times[i] - time));
		rez.x -= Qc * Dc.x * Dc.z * wa * s * attenuation_coeff * exp(-EXP_LEN*len);
		rez.z -= Qc * Dc.z * Dc.z * wa * s * attenuation_coeff * exp(-EXP_LEN*len);
		rez.y += Dc.z * wa * c * attenuation_coeff * exp(-EXP_LEN*len);
	}
	return rez;
}

vec3 get_N(vec3 position)
{
	vec3 rez = vec3(0,1,0);
	for(int i = 0; i < num_g_waves; i++)
	{
		float wa = w[i] * A[i];
		float gernst = w[i] * dot( D[i] , position) + time * phi[i];
		float s = sin(gernst);
		float c = cos(gernst);
		rez.x -= D[i].x * wa * c;
		rez.z -= D[i].z * wa * c;
		rez.y -= Q[i] * wa * s;
	}
	for(int i = 0; i < num_c_waves; i++)
	{
		vec3 Dc = vec3(0,0,0);
		float len = 0;
		vec3 col_dir = position - collision_centers[i];

		if( position != collision_centers[i] )
		{
			len = length(col_dir);
			Dc = -normalize(col_dir);
		}
		//len *= len*len;
		//if(len < 1) len = 1;


		float wa = wc * Ac;
		float gernst =  wc * dot(Dc , col_dir) + time * phic;
		float s = sin(gernst);
		float c = cos(gernst);

		float attenuation_coeff = exp(EXP_TIME*(initial_times[i] - time));
		rez.x -= Dc.x * wa * c * attenuation_coeff * exp(-EXP_LEN*len);
		rez.z -= Dc.z * wa * c * attenuation_coeff * exp(-EXP_LEN*len);
		rez.y -= Qc * wa * s * attenuation_coeff * exp(-EXP_LEN*len);
	}
	return rez;
}

void main()
{
	Qc = 0.3;
	Lc = 0.1;
	Ac = 0.04;
	Sc = 0.3;
	wc  = sqrt( 9.81 * 2 * M_PI / Lc) ;
	phic = Sc * wc;
	steepc = Qc / (wc*Ac);

	vec3 new_position =  gernster_position(v_position);

	T = get_T(new_position);
	N = get_N(new_position);
	B = get_B(new_position);

	world_position = (Model * vec4(new_position,1)).xyz;
	world_normal = normalize( mat3(Model) * N );

	texture_coord = v_texture_coord;

	gl_Position = Projection * View * Model * vec4(new_position, 1);
}