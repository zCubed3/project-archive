#include "glsl_shader.hpp"

#include <GL/glew.h>

#include <fstream>
#include <iostream>
#include <sstream>
#include <iomanip>

void LoadShaderCode(std::string path, GLSLShader::ShaderType type, std::string& output) 
{
	std::string shaderFilePath = path + GLSLShader::GetShaderTypeExtension(type);
	std::ifstream shaderFile(shaderFilePath, std::ios::in);

	if (shaderFile.is_open())
	{
		output = std::string
		(
			std::istreambuf_iterator<char>(shaderFile),
			std::istreambuf_iterator<char>()
		);

		LOG_RENDERER << "Loaded shader!\n";
		shaderFile.close();
	}
	else
		throw std::exception("Shader does not exist!");
}

void CompileShader(GLuint& shaderID, const char* pShaderSource)
{
	LOG_RENDERER << "Compiling shader...\n";

	glShaderSource(shaderID, 1, &pShaderSource, nullptr);
	glCompileShader(shaderID);

	LOG_RENDERER << "Done!\n";
}

bool CheckShader(GLuint& shaderID, std::string& output) 
{
	LOG_RENDERER << "Checking shader...\n";

	GLint result = GL_FALSE;
	int infoLogLength;

	glGetShaderiv(shaderID, GL_COMPILE_STATUS, &result);
	glGetShaderiv(shaderID, GL_INFO_LOG_LENGTH, &infoLogLength);

	if (infoLogLength > 0) 
	{
		char errorMsg[infoLogLength + 1];
		glGetShaderInfoLog(shaderID, infoLogLength, nullptr, errorMsg);

		output.append(errorMsg);
	}

	if (infoLogLength == 0)
		LOG_RENDERER << "Passed!\n";
	else
		LOG_RENDERER << "Check logs!\n";


	return result;
}

bool CheckProgram(GLuint& programID) 
{
	LOG_RENDERER << "Checking shader program...\n";

	GLint result = GL_FALSE;
	int infoLogLength = 0;

	glGetProgramiv(programID, GL_LINK_STATUS, &result);
	glGetProgramiv(programID, GL_INFO_LOG_LENGTH, &infoLogLength);

	if (infoLogLength > 0) 
	{
		char errorMsg[infoLogLength + 1];
		glGetProgramInfoLog(programID, infoLogLength, nullptr, errorMsg);

		LOG_RENDERER << errorMsg;
	}

	LOG_RENDERER << "Done!\n";
	return !result;
}

GLSLShader::GLSLShader(std::string shaderPath, std::string shaderName, std::string& outputHandler, std::string manualVertCode, std::string manualFragCode)
{
	outputHandler = "";
	path = shaderPath;
	name = shaderName;

	GLuint vertShaderID = glCreateShader(GL_VERTEX_SHADER);
	GLuint fragShaderID = glCreateShader(GL_FRAGMENT_SHADER);

	// Load the vert shader file
	std::string vertShaderCode;

	if (!shaderPath.empty())
	{
		LOG_RENDERER << "Loading vert shader from " << shaderPath << "\n";
		LoadShaderCode(path, GLSLShader::ShaderType::Vert, vertShaderCode);
	}
	else
		vertShaderCode = manualVertCode;

	// Load the frag shader file
	std::string fragShaderCode;

	if (!shaderPath.empty())
	{
		LOG_RENDERER << "Loading frag shader from " << shaderPath << "\n";
		LoadShaderCode(path, GLSLShader::ShaderType::Frag, fragShaderCode);
	}
	else
		fragShaderCode = manualFragCode;

	// Compile and check the shader
	CompileShader(vertShaderID, vertShaderCode.c_str());
	CompileShader(fragShaderID, fragShaderCode.c_str());

	didCompile = (CheckShader(vertShaderID, outputHandler) && CheckShader(fragShaderID, outputHandler));

	// Then link the shaders to the program
	if (didCompile) 
	{
		programID = glCreateProgram();

		glAttachShader(programID, vertShaderID);
		glAttachShader(programID, fragShaderID);

		glLinkProgram(programID);

		CheckProgram(programID);

		glDetachShader(programID, vertShaderID);
		glDetachShader(programID, fragShaderID);

		glDeleteShader(vertShaderID);
		glDeleteShader(fragShaderID);

		modelMatrix = glGetUniformLocation(programID, "M");
		viewMatrix = glGetUniformLocation(programID, "V");
		projectionMatrix = glGetUniformLocation(programID, "P");

		fTime = glGetUniformLocation(programID, "_time");

		std::string baseName = "light_";
		for (int l = 0; l < 32; l++)
		{
			lightPositions.push_back(glGetUniformLocation(programID, (baseName + "positions[" + std::to_string(l) + "]").c_str()));
			lightColors.push_back(glGetUniformLocation(programID, (baseName + "colors[" + std::to_string(l) + "]").c_str()));
			lightStrengths.push_back(glGetUniformLocation(programID, (baseName + "strengths[" + std::to_string(l) + "]").c_str()));
			lightRanges.push_back(glGetUniformLocation(programID, (baseName + "ranges[" + std::to_string(l) + "]").c_str()));
			lightTypes.push_back(glGetUniformLocation(programID, (baseName + "types[" + std::to_string(l) + "]").c_str()));
		}

		lightCount = glGetUniformLocation(programID, "light_count");

		viewPosition = glGetUniformLocation(programID, "camera_position");

		tex0 = glGetUniformLocation(programID, "tex0");

		colAlbedo = glGetUniformLocation(programID, "color_albedo");

		strShiny = glGetUniformLocation(programID, "gloss");

		outputHandler = "Success!\n";
	}
	else
	{
		glDeleteShader(vertShaderID);
		glDeleteShader(fragShaderID);
	}
}

GLSLShader::~GLSLShader() 
{
	glDeleteProgram(programID);
}

GLSLShader* GLSLShader::ErrorShader = nullptr;
GLSLShader* GLSLShader::UnlitShader = nullptr;

void GLSLShader::InitializeDefaults() 
{
	LOG_RENDERER << "Compiling default shader!\n";
	std::string defaultOut;

	ErrorShader = new GLSLShader("", "error", defaultOut, errorVertCode, errorFragCode);
	LOG_RENDERER << defaultOut;

	UnlitShader = new GLSLShader("", "unlit-default", defaultOut, unlitVertCode, unlitFragCode);
	LOG_RENDERER << defaultOut;
}

std::string GLSLShader::GetShaderTypeExtension(ShaderType type)
{
	switch (type) 
	{
	case ShaderType::Frag:
		return ".frag";

	case ShaderType::Vert:
		return ".vert";

	default:
		return "";
	}
}