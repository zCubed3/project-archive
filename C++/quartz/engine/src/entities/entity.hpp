#pragma once

class Model;
class Material;
class Renderer;

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

#include <string>

class Entity
{

public:
	Entity() {};
	Entity(std::string name) { this->name = name; };

	Model* model;
	Material* material;

	std::string name;

	void Draw(Renderer* renderer);

	glm::vec3 position = glm::vec3(0, 0, 0);
	glm::quat rotation = glm::quat(0, 0, 0, 0);
	glm::vec3 scale = glm::vec3(1, 1, 1);

protected:
	glm::mat4 GetMatrix();

};

