#pragma once

#include <common.hpp>

#include <vector>
#include <glm/glm.hpp>

class Camera;
class Entity;
struct Light;

struct World
{

	Camera* camera;
	std::vector<Light*> lights;
	std::vector<Entity*> entities;

	glm::vec4 clearColor = glm::vec4(0.1f, 0.1f, 0.1f, 1.0f);

	void Update();

};

