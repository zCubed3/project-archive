#pragma once

struct AgnosticFile;

#include <map>
#include <string>

class Texture
{

public:
	static Texture* Load(AgnosticFile* file);
	static Texture* Load(const char* name, unsigned char* data, int width, int height);
	static Texture* LoadBlank();
	~Texture();

	unsigned char* data;
	int width = 0, height = 0;

	const char* name;

protected:
	static std::map<std::string, Texture*> cachedTextures;

};

