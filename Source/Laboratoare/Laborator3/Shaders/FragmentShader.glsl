#version 430
layout(location = 0) in vec2 texture_coord;

uniform sampler2D texture_1;

layout(location = 0) out vec4 out_color;

float LinearizeDepth(vec2 uv)
{
  float n = 0.01; // camera z near
  float f = 00.0; // camera z far
  float z = texture2D(texture_1, uv).x;
  return (2.0 * n) / (f + n - z * (f - n));	
}

void main()
{
	vec4 color = texture(texture_1, texture_coord);
	float col;

	//col = LinearizeDepth(texture_coord);
	//color = vec4( vec3(col),1);

	if (color.a < 0.75)
	{
		discard;
	}

	out_color = color;
}
