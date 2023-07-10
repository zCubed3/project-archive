#include "material.hpp"

#include <rendering/gl_renderer/glsl/glsl_shader.hpp>

Material::Material(GLSLShader* shaderPtr)
{
	shader = GLSLShader::ErrorShader;

	if (shaderPtr != nullptr)
		if (shaderPtr->didCompile)
			shader = shaderPtr;
}
