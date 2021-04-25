#version 410

layout(location = 0) in vec2 texture_coord;

uniform sampler2D textureImage;
uniform ivec2 screenSize;
uniform int flipVertical;

// 0 - original
// 1 - grayscale
// 2 - blur
uniform int outputMode = 2;

// Flip texture horizontally when
vec2 textureCoord = vec2(texture_coord.x, (flipVertical != 0) ? 1 - texture_coord.y : texture_coord.y);

layout(location = 0) out vec4 out_color;

vec4 grayscale()
{
	vec4 color = texture(textureImage, textureCoord);
	float gray = 0.21 * color.r + 0.71 * color.g + 0.07 * color.b; 
	return vec4(gray, gray, gray,  0);
}

vec4 blur(int blurRadius)
{
	vec2 texelSize = 1.0f / screenSize;
	vec4 sum = vec4(0);
	for(int i = -blurRadius; i <= blurRadius; i++)
	{
		for(int j = -blurRadius; j <= blurRadius; j++)
		{
			sum += texture(textureImage, textureCoord + vec2(i, j) * texelSize);
		}
	}
		
	float samples = pow((2 * blurRadius + 1), 2);
	return sum / samples;
}

#define M_PI 3.1415926535897932384626433832795

vec4 gaussian(int blurRadius, float stdev)
{
	vec2 texelSize = 1.0f / screenSize;
	vec4 sum = vec4(0);
	float stdevsq = stdev * stdev;
	float impart = 2 * M_PI *  stdevsq;
	for(int i = -blurRadius; i <= blurRadius; i++)
	{
		for(int j = -blurRadius; j <= blurRadius; j++)
		{
			sum += texture(textureImage, textureCoord + vec2(i, j) * texelSize) * exp(-(i*i + j*j)/(2*stdevsq))/impart;
		}
	}
		
	return sum;
}

vec4 median()
{
	vec2 texelSize = 1.0f / screenSize;
	vec4 vals[10];

	vec4 out_color = vec4(0);
	for(int i = -1; i <= 1; i++)
	{
		for(int j = -1; j <= 1; j++)
		{
			vals[(i+1)*3 + j+ 1] = texture(textureImage, textureCoord + vec2(i, j) * texelSize);
		}
	}
	vec4 auxi;

	for( int i = 0; i < 3; i++)
		for( int j = i+1; j < 3; j++)
			if( vals[i][0] < vals[j][0])
			{
				auxi = vals[j];
				vals[j] = vals[i];
				vals[i] = auxi;
			}
	out_color.r = vals[4][0];

	for( int i = 0; i < 3; i++)
		for( int j = i+1; j < 3; j++)
			if( vals[i][1] < vals[j][1])
			{
				auxi = vals[j];
				vals[j] = vals[i];
				vals[i] = auxi;
			}
	out_color.g = vals[4][1];

	
	for( int i = 0; i < 3; i++)
		for( int j = i+1; j < 3; j++)
			if( vals[i][2] < vals[j][2])
			{
				auxi = vals[j];
				vals[j] = vals[i];
				vals[i] = auxi;
			}
	out_color.b = vals[4][2];

	return out_color;
}

void main()
{
	switch (outputMode)
	{
		case 1:
		{
			out_color = grayscale();
			break;
		}

		case 2:
		{
			out_color = blur(3);
			break;
		}

		case 3:
		{
			out_color = gaussian(3, 3);
			break;
		}

		case 4:
		{
			out_color = median();
			break;
		}

		default:
			out_color = texture(textureImage, textureCoord);
			break;
	}
}