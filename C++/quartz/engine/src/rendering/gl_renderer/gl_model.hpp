#pragma once

#include <GL/glew.h>

#include "glsl/glsl_shader.hpp"

#include "../base/model.hpp"

class GLModel : public Model
{
public:
	void Finalize() override;
	~GLModel() override;

	void Draw(Renderer* renderer, Material* material, glm::mat4 modelMatrix) override;

	GLuint vID;
	GLuint vertexBuffer;
	GLuint uvBuffer;
	GLuint indicesBuffer;
	GLuint normalBuffer;

	GLuint tex0Buffer;
};

