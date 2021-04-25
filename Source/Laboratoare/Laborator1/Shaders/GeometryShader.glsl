#version 430
layout(triangles) in;
layout(triangle_strip, max_vertices = 170) out;

layout(location = 0) in vec2 v_texture_coord[];

uniform mat4 View;
uniform mat4 Projection;

uniform int instances;
uniform float shrink;

vec3 centru_greu;

layout(location = 0) out vec2 texture_coord;

void EmitPoint(vec3 pos, vec3 offset)
{
	gl_Position = Projection * View * vec4( centru_greu + (pos - centru_greu) * shrink + offset, 1.0);
	EmitVertex();
}

void main()
{
	vec3 p1 = gl_in[0].gl_Position.xyz;
	vec3 p2 = gl_in[1].gl_Position.xyz;
	vec3 p3 = gl_in[2].gl_Position.xyz;

	centru_greu = ( p1 + p2 + p3) / 3; 

	for (int i = 0; i <= instances; i++)
	{
		//TODO modify offset so that instances are displayed on 6 columns
		vec3 offset = vec3( (i%6) * 2 ,0 , -(i/6)*2);

		//TODO modify the points so that the triangle shrinks relative to its center
		texture_coord = v_texture_coord[0];
		EmitPoint(p1, offset);

		texture_coord = v_texture_coord[1];
		EmitPoint(p2, offset);

		texture_coord = v_texture_coord[2];
		EmitPoint(p3, offset);

		EndPrimitive();
	}
}
