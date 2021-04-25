#version 430

layout(location = 0) in vec3 world_position;
layout(location = 1) in vec3 world_normal;

uniform sampler2D texture_1;
uniform samplerCube texture_cubemap;

uniform vec3 camera_position;
uniform int toggle_val;

layout(location = 0) out vec4 out_color;

vec3 myReflect()
{
    // TODO - compute the reflection color value
	vec3 incident = normalize(world_position - camera_position);
	vec3 direction = reflect( incident, world_normal);
	return texture(texture_cubemap, direction).xyz;
}

vec3 myRefract(float refractive_index)
{
    // TODO - compute the refraction color value
	return texture( texture_cubemap, refract( normalize(world_position - camera_position), world_normal, 1.0/refractive_index)).xyz;
}

void main()
{
	if(toggle_val == 0)
		out_color = vec4(myReflect(), 0);
	else 
		out_color = vec4(myRefract(1.33), 0);
}
