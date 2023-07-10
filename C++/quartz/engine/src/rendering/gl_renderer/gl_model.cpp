#include "gl_model.hpp"

#include <iostream>

#include <filesystem/agnosticfs.hpp>
#include <rendering/base/camera.hpp>

#include "../base/material.hpp"
#include "../base/light.hpp"
#include "../base/texture.hpp"

#include <glm/gtc/type_ptr.hpp>

void CreateBuffers(GLModel* model) 
{
	glGenVertexArrays(1, &model->vID);
	glBindVertexArray(model->vID);

	glGenBuffers(1, &model->vertexBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, model->vertexBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * model->vertices.size(), model->vertices.data(), GL_STATIC_DRAW);

	glGenBuffers(1, &model->uvBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, model->uvBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec2) * model->uvs.size(), model->uvs.data(), GL_STATIC_DRAW);

	glGenBuffers(1, &model->normalBuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, model->normalBuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(glm::vec3) * model->normals.size(), model->normals.data(), GL_STATIC_DRAW);

	glGenBuffers(1, &model->indicesBuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, model->indicesBuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * model->indices.size(), model->indices.data(), GL_STATIC_DRAW);

	glGenTextures(1, &model->tex0Buffer);
	glBindTexture(GL_TEXTURE_2D, model->tex0Buffer);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glBindVertexArray(0);
}

void GLModel::Finalize()
{
	CreateBuffers(this); 
}

GLModel::~GLModel()
{
	glBindVertexArray(0);

	glDeleteBuffers(1, &vertexBuffer);
	glDeleteBuffers(1, &uvBuffer);
	glDeleteBuffers(1, &normalBuffer);
	glDeleteBuffers(1, &indicesBuffer);
	
	glDeleteVertexArrays(1, &vID);
}

void GLModel::Draw(Renderer* renderer, Material* material, glm::mat4 modelMatrix)
{
	glBindVertexArray(0);

	GLSLShader* activeShader;
	activeShader = GLSLShader::ErrorShader;

	if (material != nullptr)
		if (material->shader != nullptr)
			if (material->shader->didCompile)
				activeShader = material->shader;

	if (activeShader == nullptr)
	{
		std::cout << "Shader is null!\n";
		return;
	}

	glBindVertexArray(vID);

	glUseProgram(activeShader->programID);

	glUniformMatrix4fv(activeShader->modelMatrix, 1, GL_FALSE, glm::value_ptr(modelMatrix));
	glUniformMatrix4fv(activeShader->viewMatrix, 1, GL_FALSE, glm::value_ptr(renderer->viewMatrix));
	glUniformMatrix4fv(activeShader->projectionMatrix, 1, GL_FALSE, glm::value_ptr(renderer->projectionMatrix));
	
	// Pass all the scene lights into this shader
	int lIndex = 0;
	for (auto light : renderer->world.lights)
	{
		glUniform3fv(activeShader->lightPositions[lIndex], 1, glm::value_ptr(light->position));
		glUniform3fv(activeShader->lightColors[lIndex], 1, glm::value_ptr(light->color));
		glUniform1f(activeShader->lightStrengths[lIndex], light->enabled ? light->strength : 0);
		glUniform1f(activeShader->lightRanges[lIndex], light->range);
		glUniform1i(activeShader->lightTypes[lIndex], (int)light->type);

		lIndex++;
	}

	glUniform1i(activeShader->lightCount, renderer->world.lights.size());

	if (renderer->world.camera != nullptr)
		glUniform3fv(activeShader->viewPosition, 1, glm::value_ptr(renderer->world.camera->cameraPosition));

	glUniform1f(activeShader->fTime, renderer->accumulatedTime);

	// Pass the material's textures into the shader
	//glActiveTexture(GL_TEXTURE0);

	glBindTexture(GL_TEXTURE_2D, tex0Buffer);

	Texture* blank = Texture::LoadBlank();
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, blank->width, blank->height, 0, GL_RGBA, GL_UNSIGNED_BYTE, blank->data);
	if (material != nullptr) 
	{
		glUniform4fv(activeShader->colAlbedo, 1, glm::value_ptr(material->albedoColor));
		glUniform1f(activeShader->strShiny, material->gloss);

		if (material->tex0 != nullptr)
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, material->tex0->width, material->tex0->height, 0, GL_RGBA, GL_UNSIGNED_BYTE, material->tex0->data);
	}

	//glUniform1i(activeShader->tex0, 0);

	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
	glVertexAttribPointer
	(
		0,
		3,
		GL_FLOAT,
		GL_FALSE,
		0,
		(void*)0
	);

	glEnableVertexAttribArray(1);

	glBindBuffer(GL_ARRAY_BUFFER, uvBuffer);
	glVertexAttribPointer
	(
		1,
		2,
		GL_FLOAT,
		GL_FALSE,
		0,
		(void*)0
	);

	glEnableVertexAttribArray(2);

	glBindBuffer(GL_ARRAY_BUFFER, normalBuffer);
	glVertexAttribPointer
	(
		2,
		3,
		GL_FLOAT,
		GL_FALSE,
		0,
		(void*)0
	);

	glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, (void*)0);

	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);
	glDisableVertexAttribArray(2);
}