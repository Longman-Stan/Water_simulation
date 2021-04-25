#include "Laborator7.h"

#include <vector>
#include <iostream>

#include <Core/Engine.h>

using namespace std;

// Order of function calling can be seen in "Source/Core/World.cpp::LoopUpdate()"
// https://github.com/UPB-Graphics/SPG-Framework/blob/master/Source/Core/World.cpp

Laborator7::Laborator7()
{
	outputMode = 0;
	gpuProcessing = false;
	saveScreenToImage = false;
	window->SetSize(600, 600);
}

Laborator7::~Laborator7()
{
}

FrameBuffer *processed;

void Laborator7::Init()
{
	// Load default texture fore imagine processing 
	originalImage = TextureManager::LoadTexture(RESOURCE_PATH::TEXTURES + "Cube/posx.png", nullptr, "image", true, true);
	processedImage = TextureManager::LoadTexture(RESOURCE_PATH::TEXTURES + "Cube/posx.png", nullptr, "newImage", true, true);

	{
		Mesh* mesh = new Mesh("quad");
		mesh->LoadMesh(RESOURCE_PATH::MODELS + "Primitives", "quad.obj");
		mesh->UseMaterials(false);
		meshes[mesh->GetMeshID()] = mesh;
	}

	static std::string shaderPath = "Source/Laboratoare/Laborator7/Shaders/";

	// Create a shader program for particle system
	{
		Shader *shader = new Shader("ImageProcessing");
		shader->AddShader((shaderPath + "VertexShader.glsl").c_str(), GL_VERTEX_SHADER);
		shader->AddShader((shaderPath + "FragmentShader.glsl").c_str(), GL_FRAGMENT_SHADER);

		shader->CreateAndLink();
		shaders[shader->GetName()] = shader;
	}
}

void Laborator7::FrameStart()
{
}

void Laborator7::Update(float deltaTimeSeconds)
{
	ClearScreen();

	auto shader = shaders["ImageProcessing"];
	shader->Use();

	if (saveScreenToImage)
	{
		window->SetSize(originalImage->GetWidth(), originalImage->GetHeight());
	}

	int flip_loc = shader->GetUniformLocation("flipVertical");
	glUniform1i(flip_loc, saveScreenToImage ? 0 : 1);

	int screenSize_loc = shader->GetUniformLocation("screenSize");
	glm::ivec2 resolution = window->GetResolution();
	glUniform2i(screenSize_loc, resolution.x, resolution.y);

	int outputMode_loc = shader->GetUniformLocation("outputMode");
	glUniform1i(outputMode_loc, outputMode);

	int gpuProcessing_loc = shader->GetUniformLocation("outputMode");
	glUniform1i(outputMode_loc, outputMode);

	int locTexture = shader->GetUniformLocation("textureImage");
	glUniform1i(locTexture, 0);
	auto textureImage = (gpuProcessing == true) ? originalImage : processedImage;
	textureImage->BindToTextureUnit(GL_TEXTURE0);

	RenderMesh(meshes["quad"], shader, glm::mat4(1));

	if (saveScreenToImage)
	{
		saveScreenToImage = false;
		GLenum format = GL_RGB;
		if (originalImage->GetNrChannels() == 4)
		{
			format = GL_RGBA;
		}
		glReadPixels(0, 0, originalImage->GetWidth(), originalImage->GetHeight(), format, GL_UNSIGNED_BYTE, processedImage->GetImageData());
		processedImage->UploadNewData(processedImage->GetImageData());
		SaveImage("shader_processing_" + std::to_string(outputMode));

		float aspectRatio = static_cast<float>(originalImage->GetWidth()) / originalImage->GetHeight();
		window->SetSize(static_cast<int>(600 * aspectRatio), 600);
	}
}

void Laborator7::FrameEnd()
{
	DrawCoordinatSystem();
}

void Laborator7::OnFileSelected(std::string fileName)
{
	if (fileName.size())
	{
		std::cout << fileName << endl;
		originalImage = TextureManager::LoadTexture(fileName.c_str(), nullptr, "image", true, true);
		processedImage = TextureManager::LoadTexture(fileName.c_str(), nullptr, "newImage", true, true);

		float aspectRatio = static_cast<float>(originalImage->GetWidth()) / originalImage->GetHeight();
		window->SetSize(static_cast<int>(600 * aspectRatio), 600);
	}
}

void Laborator7::GrayScale()
{
	unsigned int channels = originalImage->GetNrChannels();
	unsigned char* data = originalImage->GetImageData();
	unsigned char* newData = processedImage->GetImageData();

	if (channels < 3)
		return;

	glm::ivec2 imageSize = glm::ivec2(originalImage->GetWidth(), originalImage->GetHeight());

	for (int i = 0; i < imageSize.y; i++)
	{
		for (int j = 0; j < imageSize.x; j++)
		{
			int offset = channels * (i * imageSize.x + j);

			// Reset save image data
			char value = static_cast<char>(data[offset + 0] * 0.2f + data[offset + 1] * 0.71f + data[offset + 2] * 0.07);
			memset(&newData[offset], value, 3);
		}
	}

	processedImage->UploadNewData(newData);
}

void Laborator7::Medie(int blurRadius)
{
	unsigned int channels = originalImage->GetNrChannels();
	unsigned char* data = originalImage->GetImageData();
	unsigned char* newData = processedImage->GetImageData();

	if (channels < 3)
		return;

	processedImage->UploadNewData(data);

	glm::ivec2 imageSize = glm::ivec2(originalImage->GetWidth(), originalImage->GetHeight());
	int kern_size = (blurRadius*2+1) * (blurRadius * 2 + 1);

	for (int y = blurRadius; y < imageSize.y - blurRadius; y++)
		for (int x = blurRadius; x < imageSize.x - blurRadius; x++)
		{
			int sumr, sumg, sumb;
			sumr = sumg = sumb = 0;
			int off = channels * ( y * imageSize.x + x);

			for (int i = -blurRadius; i <= blurRadius; i++)
				for (int j = -blurRadius; j <= blurRadius; j++)
				{
					int offset = channels * ( (y+i) * imageSize.x + (x+j));
					sumr += data[offset + 0];
					sumg += data[offset + 1];
					sumb += data[offset + 2];
				}

			newData[off] = (char)(sumr / kern_size);
			newData[off+1] = (char)(sumg / kern_size);
			newData[off+2] = (char)(sumb / kern_size);
			// Reset save image data
		}

	processedImage->UploadNewData(newData);
}

void Laborator7::Gaussian(int Radius, float stdev)
{
	unsigned int channels = originalImage->GetNrChannels();
	unsigned char* data = originalImage->GetImageData();
	unsigned char* newData = processedImage->GetImageData();

	if (channels < 3)
		return;

	processedImage->UploadNewData(data);
	glm::ivec2 imageSize = glm::ivec2(originalImage->GetWidth(), originalImage->GetHeight());
	float stdevsq = stdev * stdev;
	float impart = 2 * M_PI * stdevsq;

	for (int y = Radius; y < imageSize.y - Radius; y++)
		for (int x = Radius; x < imageSize.x - Radius; x++)
		{
			int off = channels * (y * imageSize.x + x);
			float r, g, b;
			r = g = b = 0;

			for (int i = -Radius; i <= Radius; i++)
				for (int j = -Radius; j <= Radius; j++)
				{
					int offset = channels * ((y + i) * imageSize.x + (x + j));
					float gauss_filter = exp( -(float)((i * i + j * j)) / (2 * stdevsq) );
					r += ((float)data[offset]) *gauss_filter;
					g += ((float)data[offset + 1]) *gauss_filter;
					b += ((float)data[offset + 2]) *gauss_filter;
				}

			r /= impart;
			g /= impart;
			b /= impart;

			if (r < 0) r = 0; 
			if (r > 255) r = 255;
			if (g < 0) g = 0;
			if (g > 255) g = 255;
			if (b < 0) b = 0;
			if (b > 255) b = 255;

			newData[off] = (unsigned char)(r);
			newData[off + 1] = (unsigned char)(g);
			newData[off + 2] = (unsigned char)(b);

		}


	processedImage->UploadNewData(newData);
}

void Laborator7::Median()
{
	unsigned int channels = originalImage->GetNrChannels();
	unsigned char* data = originalImage->GetImageData();
	unsigned char* newData = processedImage->GetImageData();

	if (channels < 3)
		return;

	processedImage->UploadNewData(data);
	glm::ivec2 imageSize = glm::ivec2(originalImage->GetWidth(), originalImage->GetHeight());
	char r[10];
	char g[10];
	char b[10];

	for (int y = 1; y < imageSize.y - 1; y++)
		for (int x = 1; x < imageSize.x - 1; x++)
		{
			int off = channels * (y * imageSize.x + x);

			for (int i = -1; i <= 1; i++)
				for (int j = -1; j <= 1; j++)
				{
					int offset = channels * ((y + i) * imageSize.x + (x + j));
					r[(i+1) * 3 + j+1] = data[offset];
					g[(i+1) * 3 + j+1] = data[offset+1];
					b[(i+1) * 3 + j+1] = data[offset+2];
				}
			for (int i = 0; i <= 2; i++)
				for (int j = i+1; j <= 2; j++)
				{
					char auxi;
					if (r[i] > r[j])
					{
						auxi = r[j];
						r[j] = r[i];
						r[i] = auxi;
					}
					if (g[i] > g[j])
					{
						auxi = g[j];
						g[j] = g[i];
						g[i] = auxi;
					}
					if (b[i] > b[j])
					{
						auxi = b[j];
						b[j] = b[i];
						b[i] = auxi;
					}
				}

			newData[off] = (char)(r[4]);
			newData[off + 1] = (char)(g[4]);
			newData[off + 2] = (char)(b[4]);
		}
	processedImage->UploadNewData(newData);
}

void Laborator7::Roberts()
{
	cout << "Roberts\n";
	GrayScale();

	unsigned int channels = originalImage->GetNrChannels();
	unsigned char* data = processedImage->GetImageData();
	unsigned char* newData = processedImage->GetImageData();

	if (channels < 3)
		return;

	glm::ivec2 imageSize = glm::ivec2(originalImage->GetWidth(), originalImage->GetHeight());
	int val;
	unsigned char p1, p2, p3, p4;

	for (int y = imageSize.y; y >= 1; y--)
		for (int x = 0; x < imageSize.x - 1; x++)
		{
			int off = channels * (y * imageSize.x + x);
			p1 = data[channels * ((y-1) * imageSize.x + x)];
			p2 = data[channels * ((y-1) * imageSize.x + x + 1)];
			p3 = data[channels * (y * imageSize.x + x)];
			p4 = data[channels * (y * imageSize.x + x + 1)];

			newData[off] = (char)(abs(p1 - p4) + abs(p2 - p3));
			newData[off + 1] = newData[off];
			newData[off + 2] = newData[off];
		}

	processedImage->UploadNewData(newData);
}

void Laborator7::Sobel()
{
	cout << "Sobel\n";
	GrayScale();

	unsigned int channels = originalImage->GetNrChannels();
	unsigned char* data = processedImage->GetImageData();
	unsigned char* newData = processedImage->GetImageData();

	if (channels < 3)
		return;

	glm::ivec2 imageSize = glm::ivec2(originalImage->GetWidth(), originalImage->GetHeight());

	int sz = originalImage->GetHeight() * originalImage->GetWidth();

	unsigned char* gray = (unsigned char*)malloc( channels * sz * sizeof(unsigned char));
	memcpy(gray, data, channels * sz * sizeof(unsigned char));
	
	int val;
	int r[10];
	char g[10];
	char b[10];
	int v1, v2;

	for (int y = 1; y < imageSize.y - 1; y++)
		for (int x = 1; x < imageSize.x - 1; x++)
		{

			int off = channels * (y * imageSize.x + x);

			for (int i = -1; i <= 1; i++)
				for (int j = -1; j <= 1; j++)
				{
					int offset = channels * ((y + i) * imageSize.x + (x + j));
					r[(i + 1) * 3 + j + 1] = gray[offset];
				}

			v1 = (int)r[2] + (int)2 * r[5] + (int)r[8] - (int)r[0] - 2 * (int)r[3] - (int)r[6];
			v2 = (int)r[0] + 2 * (int)r[1] + (int)r[2] - (int)r[6] - 2 * (int)r[7] - (int)r[8];
			newData[off] = (char)(abs(v1)+ abs(v2));

			newData[off + 1] = newData[off];
			newData[off + 2] = newData[off];
		}

	processedImage->UploadNewData(newData);
}

void Laborator7::Prewitt()
{
	cout << "Prewitt\n";
	GrayScale();

	unsigned int channels = originalImage->GetNrChannels();
	unsigned char* data = processedImage->GetImageData();
	unsigned char* newData = processedImage->GetImageData();

	int sz = originalImage->GetHeight() * originalImage->GetWidth();
	unsigned char* gray = (unsigned char*)malloc(channels * sz * sizeof(unsigned char));
	memcpy(gray, data, channels * sz * sizeof(unsigned char));

	if (channels < 3)
		return;

	glm::ivec2 imageSize = glm::ivec2(originalImage->GetWidth(), originalImage->GetHeight());
	int val;
	int r[10];
	char g[10];
	char b[10];

	for (int y = 1; y < imageSize.y - 1; y++)
		for (int x = 1; x < imageSize.x - 1; x++)
		{

			int off = channels * (y * imageSize.x + x);

			for (int i = -1; i <= 1; i++)
				for (int j = -1; j <= 1; j++)
				{
					int offset = channels * ((y + i) * imageSize.x + (x + j));
					r[(i + 1) * 3 + j + 1] = gray[offset];
				}

			newData[off] = (char)(abs(r[2] + r[5] + r[8] - r[0] - r[3] - r[6]) + abs(r[0] + r[1] + r[2] - r[6] - r[7] - r[8]));
			newData[off + 1] = newData[off];
			newData[off + 2] = newData[off];
		}
	processedImage->UploadNewData(newData);
}

void Laborator7::Laplace()
{
	cout << "Laplace\n";
	GrayScale();

	unsigned int channels = originalImage->GetNrChannels();
	unsigned char* data = processedImage->GetImageData();
	unsigned char* newData = processedImage->GetImageData();

	int sz = originalImage->GetHeight() * originalImage->GetWidth();
	int* intensity = (int*)malloc(channels * sz * sizeof(int));

	if (channels < 3)
		return;

	glm::ivec2 imageSize = glm::ivec2(originalImage->GetWidth(), originalImage->GetHeight());
	int val;
	int r[10];
	char g[10];
	char b[10];

	int kern[10] = { 0,1,0,1,-4,1,0,1,0 };

	for (int y = 1; y < imageSize.y - 1; y++)
		for (int x = 1; x < imageSize.x - 1; x++)
		{

			int off = channels * (y * imageSize.x + x);
			val = 0;

			for (int i = -1; i <= 1; i++)
				for (int j = -1; j <= 1; j++)
				{
					int offset = channels * ((y + i) * imageSize.x + (x + j));
					val += kern[(i+1)*3+j+1]*data[offset];
				}

			intensity[off] = val;
			intensity[off + 1] = newData[off];
			intensity[off + 2] = newData[off];
		}

	int min = 0x7fffffff;
	int max = -0x7fffffff;
	int prag = 20;

	for (int y = 1; y < imageSize.y - 1; y++)
		for (int x = 1; x < imageSize.x - 1; x++)
		{
			int off = channels * (y * imageSize.x + x);
			min = max = newData[off];

			for (int i = -1; i <= 1; i++)
				for (int j = -1; j <= 1; j++)
				{
					int offset = channels * ((y + i) * imageSize.x + (x + j));
					val = intensity[off];
					if (val < min) min = val;
					if (val > max) max = val;
				}

			if (min < -prag && max > prag)
			{
				newData[off] = (unsigned char)255;
				newData[off+1] = (unsigned char)255;
				newData[off+2] = (unsigned char)255;
			}
			else
			{
				newData[off] = 0;
				newData[off+1] = 0;
				newData[off+2] = 0;
			}
		}
	
	processedImage->UploadNewData(newData);
}

void Laborator7::SaveImage(std::string fileName)
{
	cout << "Saving image! ";
	processedImage->SaveToFile((fileName + ".png").c_str());
	cout << "[Done]" << endl;
}

// Read the documentation of the following functions in: "Source/Core/Window/InputController.h" or
// https://github.com/UPB-Graphics/SPG-Framework/blob/master/Source/Core/Window/InputController.h

void Laborator7::OnInputUpdate(float deltaTime, int mods)
{
	// treat continuous update based on input
};

void Laborator7::OnKeyPress(int key, int mods)
{
	// add key press event
	if (key == GLFW_KEY_F || key == GLFW_KEY_ENTER || key == GLFW_KEY_SPACE)
	{
		OpenDialogue();
	}

	if (key == GLFW_KEY_E)
	{
		gpuProcessing = !gpuProcessing;
		if (gpuProcessing == false)
		{
			outputMode = 0;
		}
		cout << "Processing on GPU: " << (gpuProcessing ? "true" : "false") << endl;
	}

	if (key - GLFW_KEY_0 >= 0 && key <= GLFW_KEY_9)
	{
		outputMode = key - GLFW_KEY_0;
		int out_m = outputMode;
		cout << out_m << '\n';
		if (gpuProcessing == false)
		{
			outputMode = 0;

			if(out_m == 1)
				GrayScale();
			if (out_m == 2)
				Medie(1);
			if (out_m == 3)
				Gaussian(2, 1);
			if (out_m == 4)
				Median();
			if (out_m == 5)
				Roberts();
			if (out_m == 6)
				Sobel();
			if (out_m == 7)
				Prewitt();
			if (out_m == 8)
				Laplace();
		}
	}

	if (key == GLFW_KEY_S && mods & GLFW_MOD_CONTROL)
	{
		if (!gpuProcessing)
		{
			SaveImage("processCPU_" + std::to_string(outputMode));
		}
		else
		{
			saveScreenToImage = true;
		}
	}
};

void Laborator7::OnKeyRelease(int key, int mods)
{
	// add key release event
};

void Laborator7::OnMouseMove(int mouseX, int mouseY, int deltaX, int deltaY)
{
	// add mouse move event
};

void Laborator7::OnMouseBtnPress(int mouseX, int mouseY, int button, int mods)
{
	// add mouse button press event
};

void Laborator7::OnMouseBtnRelease(int mouseX, int mouseY, int button, int mods)
{
	// add mouse button release event
}

void Laborator7::OnMouseScroll(int mouseX, int mouseY, int offsetX, int offsetY)
{
	// treat mouse scroll event
}

void Laborator7::OnWindowResize(int width, int height)
{
	// treat window resize event
}
