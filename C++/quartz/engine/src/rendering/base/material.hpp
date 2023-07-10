#pragma once

#include <common.hpp>
#include <map>

#include <glm/glm.hpp>

struct GLSLShader;

class Texture;

class Material
{

public:
	Material(GLSLShader* shaderPtr);

	Texture* tex0 = nullptr;

	glm::vec4 albedoColor = glm::vec4(1, 1, 1, 1);
	float gloss = 0;

	GLSLShader* shader;
};

