#include "TextureManager.h"

#include <include/utils.h>
#include <Core/GPU/Texture2D.h>
#include <Core/Managers/ResourcePath.h>

using namespace std;

std::unordered_map<std::string, Texture2D*> TextureManager::mapTextures;
std::vector<Texture2D*> TextureManager::vTextures;

void TextureManager::Init()
{
	LoadTexture(RESOURCE_PATH::TEXTURES, "default.png");
	LoadTexture(RESOURCE_PATH::TEXTURES, "white.png");
	LoadTexture(RESOURCE_PATH::TEXTURES, "black.jpg");
	LoadTexture(RESOURCE_PATH::TEXTURES, "noise.png");
	LoadTexture(RESOURCE_PATH::TEXTURES, "random.jpg");
	LoadTexture(RESOURCE_PATH::TEXTURES, "particle.png");
}

//TextureManager::~TextureManager()
//{
//	// delete textures
//	unsigned int size = (unsigned int) vTextures.size();
//	for (unsigned int i=0; i <size; ++i)
//		SAFE_FREE(vTextures[i]);
//}

Texture2D* TextureManager::LoadTexture(const string &path, const char *fileName, const char *key, bool forceLoad, bool cacheInRAM)
{
	std::string uid = key ? key : fileName;
	Texture2D *texture = GetTexture(uid.c_str());

	if (forceLoad || texture == nullptr)
	{
		if (texture == nullptr)
		{
			texture = new Texture2D();
		}
		
		texture->CacheInMemory(cacheInRAM);
		bool status = texture->Load2D((path + (fileName ? (string("/") + fileName) : "")).c_str());

		if (status == false)
		{
			delete texture;
			return vTextures[0];
		}

		vTextures.push_back(texture);
		mapTextures[key ? key : fileName] = texture;
	}
	return texture;
}

void TextureManager::SetTexture(string name, Texture2D *texture)
{
	mapTextures[name] = texture;
}

Texture2D* TextureManager::GetTexture(const char* name)
{
	if (mapTextures[name])
		return mapTextures[name];
	return NULL;
}

Texture2D* TextureManager::GetTexture(unsigned int textureID)
{
	if (textureID < vTextures.size())
		return vTextures[textureID];
	return NULL;
}
