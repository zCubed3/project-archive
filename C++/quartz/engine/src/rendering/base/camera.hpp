#pragma once

#include <glm/common.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "../gl_renderer/gl_renderer.hpp"

class Camera
{

public:
	Camera(GLRenderer* instance);
	~Camera();

	void Update(float deltaTime, glm::vec2 cursorVelocity, glm::vec2 windowDimensions);

	glm::mat4 camMatrix;
	glm::mat4 projMatrix;

	float xMovement;
	float yMovement;
	float zMovement;

	float fieldOfView = 70.0f;
	float nearCull = 0.1f;
	float farCull = 100.0f;
	bool orthographic = false;

	GLRenderer* creator;

	glm::vec3 cameraPosition = glm::vec3(0, 0, 3);

private:
	float timerDelay = 0;

	float pitch, yaw = -90;
	glm::vec3 cameraForward = glm::vec3(0, 0, 0);
	
	glm::vec3 cameraUp = glm::vec3(0, 1, 0);

	int bindingIndex;

public:
	bool lockCursor = false;

};

