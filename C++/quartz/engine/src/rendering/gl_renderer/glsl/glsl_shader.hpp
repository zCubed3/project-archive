#pragma once

#include <common.hpp>
#include <GL/glew.h>

#include <string>
#include <vector>

typedef unsigned int GLuint;

struct GLSLShader
{
	GLSLShader(std::string shaderPath, std::string shaderName, std::string& outputHandler, std::string manualVertCode = "", std::string manualFragCode = "");
	~GLSLShader();


	GLuint fTime;
	GLuint modelMatrix, viewMatrix, projectionMatrix;

	std::vector<GLuint> lightPositions, lightColors, lightStrengths, lightRanges, lightTypes;
	GLuint lightCount;

	GLuint viewPosition;

	GLuint tex0;

	GLuint colAlbedo;
	GLuint strShiny;

	GLuint programID;

	bool didCompile = false;

	static GLSLShader* ErrorShader;
	static GLSLShader* UnlitShader;
	
	static void InitializeDefaults();

	enum ShaderType 
	{
		Vert,
		Frag
	};

	static std::string GetShaderTypeExtension(ShaderType type);

	std::string path = "", name = "";

};

#pragma region ERROR SHADER

const std::string errorVertCode =
R"(
#version 330 core

layout(location = 0) in vec3 _pos;

uniform mat4 M;
uniform mat4 V;
uniform mat4 P;

out vec3 pos;

void main() { gl_Position = (P * V * M) * vec4(_pos, 1); pos = _pos; }
)";

const std::string errorFragCode =
R"(
#version 330 core

out vec4 color;
uniform float _time;

in vec3 pos;

void main() { color = vec4(vec3(1.0, 0.5, 0) * abs(sin(_time)), 1); }
)";

#pragma endregion

#pragma region UNLIT SHADER

const std::string unlitVertCode =
R"(
#version 330 core

layout(location = 0) in vec3 _pos;
layout(location = 1) in vec2 _uv;

uniform mat4 M;
uniform mat4 V;
uniform mat4 P;

out vec3 pos;
out vec2 uv;

void main() { gl_Position = (P * V * M) * vec4(_pos, 1); pos = _pos; uv = _uv; }
)";

const std::string unlitFragCode =
R"(
#version 330 core

out vec4 color;
uniform sampler2D tex0;

in vec3 pos;
in vec2 uv;

uniform vec4 color_albedo;

void main() { color = texture(tex0, uv) * color_albedo; }
)";

#pragma endregion

