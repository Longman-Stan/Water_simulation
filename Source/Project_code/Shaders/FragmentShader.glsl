#version 430
layout(location = 0) in vec2 texture_coord;
layout(location = 1) in vec3 T;
layout(location = 2) in vec3 N;
layout(location = 3) in vec3 B;
layout(location = 4) in vec3 world_position;
layout(location = 5) in vec3 world_normal;

uniform sampler2D texture_1;

// Uniforms for light properties
uniform vec3 light_direction;
uniform vec3 light_position;
uniform vec3 eye_position;

uniform float material_kd;
uniform float material_ks;
uniform int material_shininess;

layout(location = 0) out vec4 out_color;

uniform samplerCube texture_cubemap;

//Uniform texture normal
uniform sampler2D texture_water;

vec3 myReflect(vec3 incident, vec3 normal)
{
	vec3 direction = reflect( incident, normal);
	return texture(texture_cubemap, direction).xyz;
}

vec3 myRefract(vec3 incident, vec3 normal,float refractive_index)
{
    // TODO - compute the refraction color value
	return texture( texture_cubemap, refract( incident, normal, 1.0/refractive_index)).xyz;
}

void main()
{
	vec3 world_norm = normalize(world_normal);

	vec3 L = normalize( light_position - world_position );
	vec3 V = normalize( eye_position - world_position );
	vec3 H = normalize( L + V );

	
	mat3 TBN = mat3(T,B,N);
	vec3 norm2 = texture(texture_water, texture_coord).rgb;
	norm2 = norm2 * 2.0 - 1.0;
	norm2 = normalize( TBN * norm2);
	world_norm = normalize( (3*world_norm + norm2)/4);

	float ambient_light = 0.7;
	float diffuse_light = material_kd * max ( dot(world_norm,L), 0);

	float primesteLumina;
	if( dot(world_normal,L) > 0 )
		primesteLumina = 1;
	else 
		primesteLumina = 0;

	float specular_light = 0;

	float d = distance(world_position,light_position);
	float atenuation = 1;

	float light=ambient_light;

	if (diffuse_light > 0)
	{
		specular_light = material_ks * primesteLumina * pow(max(dot(world_norm, H), 0), material_shininess);
	}

	light += atenuation*diffuse_light;
	light += atenuation*specular_light;

	vec3 reflect_color = myReflect(-V, world_norm);
	vec3 refract_color = myRefract(-V, world_norm, 1.33);

	//out_color = texture(texture_water, texture_coord);
	out_color = vec4( (vec3(0,0.53,0.55)*0.6 + reflect_color*0.2 + refract_color*0.2) * light, 0.8);
}
