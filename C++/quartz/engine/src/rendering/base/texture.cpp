#include "texture.hpp"
#include <filesystem/agnosticfs.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

auto Texture::cachedTextures = std::map<std::string, Texture*>();

Texture* Texture::Load(AgnosticFile* file)
{
	for (auto const& [key, value] : Texture::cachedTextures)
		if (key == file->fileName)
			return value;

	int components;
	Texture* allocatedTexture = new Texture();
	stbi_set_flip_vertically_on_load(true);

	allocatedTexture->name = file->fileName.c_str();
	allocatedTexture->data = stbi_load(file->filePath.c_str(), &allocatedTexture->width, &allocatedTexture->height, &components, 4);

	cachedTextures.insert(std::make_pair(file->fileName, allocatedTexture));

	return allocatedTexture;
}

Texture* Texture::Load(const char* name, unsigned char* data, int width, int height) 
{
	for (auto const& [key, value] : Texture::cachedTextures)
		if (key == std::string(name))
			return value;

	Texture* allocatedTexture = new Texture();

	allocatedTexture->name = name;
	allocatedTexture->data = data;
	allocatedTexture->width = width;
	allocatedTexture->height = height;

	cachedTextures.insert(std::make_pair(std::string(allocatedTexture->name), allocatedTexture));
	return allocatedTexture;
}

Texture::~Texture() 
{

}

unsigned char float1Hex = 255u;
unsigned char float0Hex = 0u;

unsigned char blankData[] = 
{ 
	float1Hex, float1Hex, float1Hex, float1Hex, 
	float1Hex, float1Hex, float1Hex, float1Hex, 
	float1Hex, float1Hex, float1Hex, float1Hex, 
	float1Hex, float1Hex, float1Hex, float1Hex 
};

Texture* Texture::LoadBlank() { return Texture::Load("internal-blank", blankData, 2, 2); }