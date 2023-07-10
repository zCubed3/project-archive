#include "model.hpp"

#include <glm/gtx/transform.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <rendering/gl_renderer/gl_model.hpp>

#include <filesystem/agnosticfs.hpp>


auto Model::cachedModels = std::map<const char*, Model*>();

void Model::InitializePrimitives() 
{
	primQuad = Model::Load<GLModel>(Primitives::Quad);
}

Model* Model::primQuad = nullptr;
Model* Model::primTriangle = nullptr;