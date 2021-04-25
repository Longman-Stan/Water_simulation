#version 430
layout(lines) in;
//TODO 
//prima data generati o curba (cu line strip), apoi o suprafata de rotatie/translatie cu triangle_strip
layout(line_strip, max_vertices = 256) out;

uniform mat4 View;
uniform mat4 Projection;
uniform vec3 control_p1, control_p2, control_p3, control_p4;
uniform int no_of_instances;
uniform int no_of_generated_points;
uniform int curve_type;
uniform int show_support_points;


in int instance[2];


vec3 rotateY(vec3 point, float u)
{
	float x = point.x * cos(u) - point.z *sin(u);
	float z = point.x * sin(u) + point.z *cos(u);
	return vec3(x, point.y, z);
}

vec3 translateX(vec3 point, float t)
{
	return vec3(point.x + t, point.y, point.z);
}

vec3 bezier(float t)
{
	return control_p1 * pow((1 - t), 3) + control_p2 * 3 * t * pow((1 - t), 2) + control_p3 * 3 * pow(t, 2) * (1 - t) + control_p4 * pow(t, 3);
}

vec3 hermite(float t)
{
	float t3 = pow(t,3);
	float t2 = pow(t,2);

	return control_p1 * ( 2*t3 - 3 * t2 + 1) + control_p4 * ( -2 * t3 + 3 * t2) + (control_p2 - control_p1) * ( t3 - 2 * t2 + t) + (control_p3 - control_p4) * ( t3 - t2);
}

//TODO - incercati sa creati si o curba Hermite/Bspline

void main()
{
	float t;
	vec3 point;

	if (instance[0] < no_of_instances)
	{
		//TODO 
		//in loc sa emiteti varfuri reprezentand punctele de control, emiteti varfuri care sa aproximeze curba Bezier
		for(int i=0; i<= no_of_generated_points;i++)
		{
			t = float(i) / no_of_generated_points;

			switch( curve_type)
			{
				case 0:
					point = bezier(t);
					break;
				case 1:
					point = hermite(t);
					break;
			}				
			point = translateX(point, instance[0] * 0.5);
			point = rotateY(point, instance[0] * 0.005);
			gl_Position = Projection* View * vec4( point, 1);	EmitVertex();
		}	
		EndPrimitive();

		if( show_support_points == 1)
		{
			gl_Position = Projection* View * vec4(control_p1, 1);	EmitVertex();
			gl_Position = Projection* View * vec4(control_p2, 1);	EmitVertex();
			gl_Position = Projection* View * vec4(control_p3, 1);	EmitVertex();
			gl_Position = Projection* View * vec4(control_p4, 1);	EmitVertex();
		}
		EndPrimitive();
	}
}
