#version 430

layout(location = 0) in vec3 v_position;
layout(location = 1) in vec3 v_normal;
layout(location = 2) in vec2 v_texture_coord;

// Uniform properties
uniform mat4 Model;
uniform mat4 View;
uniform mat4 Projection;

uniform float time;
uniform float time0;
uniform int num_gerstner_base_waves;


layout(location = 0) out vec2 texture_coord;
float Q;
float L;
float A;
vec3 D1;
vec3 D2;
float S;
float steepness;
float w;
float phi;


vec3 gernster_position()
{
	vec3 new_vec;
	vec3 D = vec3(0,0,0);
	float len = 0;

	if(v_position != vec3(0,0,0))
	{
		len = length(v_position);
		D = -normalize( v_position);
	}
	float gernst1 = w * dot(D , v_position) + time * phi;
	float val_cos1 = cos(gernst1);
	float attenuation_coeff = exp((time0 - time)/5);

	new_vec.x = v_position.x + steepness * A * D.x * val_cos1 * attenuation_coeff;
	new_vec.z = v_position.z + steepness * A * D.z * val_cos1 * attenuation_coeff;
	new_vec.y = A * sin(gernst1) * 2 * exp(-len) * attenuation_coeff;// / exp(len/50);
	return new_vec;
}

void main()
{
	Q = 0.1;
	L = 0.1;
	A = 0.1;
	D1 = vec3(1,0,0.6);
	D2 = vec3(1,0,-0.6);
	S = 0.3;

	w  = 2 / L;
	phi = S * w;

	steepness = Q / (w*A);

	vec3 new_position =  gernster_position();

	texture_coord = v_texture_coord;

	gl_Position = Projection * View * Model * vec4(new_position, 1);
}