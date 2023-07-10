#pragma once

#include <string>
#include <vector>

#include <glm/matrix.hpp>

class Model;
struct Light;

#include "world.hpp"

class Renderer
{

public:
	virtual void Update() = 0;

	World world;

	float accumulatedTime = 0.0f;
	float deltaTime;

	bool active = false;

	glm::mat4 viewMatrix, projectionMatrix;
};

