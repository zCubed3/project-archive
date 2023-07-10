#include <iostream>

#include "camera.hpp"
#include <GLFW/glfw3.h>

static Camera* camera;

static void KeyDelegate(GLFWwindow* window, int key, int action) 
{
	if (camera != nullptr)
	{
		camera->xMovement = 0;
		if (glfwGetKey(window, GLFW_KEY_A))
			camera->xMovement = 1;

		if (glfwGetKey(window, GLFW_KEY_D))
			camera->xMovement = -1;

		camera->zMovement = 0;
		if (glfwGetKey(window, GLFW_KEY_W))
			camera->zMovement = 1;

		if (glfwGetKey(window, GLFW_KEY_S))
			camera->zMovement = -1;

		camera->yMovement = 0;
		if (glfwGetKey(window, GLFW_KEY_E))
			camera->yMovement = 1;

		if (glfwGetKey(window, GLFW_KEY_Q))
			camera->yMovement = -1;

		if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) 
		{
			camera->lockCursor = !camera->lockCursor;
			glfwSetInputMode(window, GLFW_CURSOR, camera->lockCursor ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL);
		}
	}
}

Camera::Camera(GLRenderer* instance)
{
	creator = instance;

	instance->keyDelegates.push_back(&KeyDelegate);
	bindingIndex = instance->keyDelegates.size() - 1;

	camera = this;

	xMovement = 0;
	yMovement = 0;
	zMovement = 0;

	pitch = 0;
	yaw = -90;

	camMatrix = glm::lookAt(cameraPosition, glm::vec3(0, 0, -1), cameraUp);
}

Camera::~Camera() 
{
	creator->keyDelegates.erase(creator->keyDelegates.begin() + bindingIndex);
}

void Camera::Update(float deltaTime, glm::vec2 cursorVelocity, glm::vec2 windowDimensions) 
{
	timerDelay += deltaTime;

	if (lockCursor) 
	{
		pitch -= cursorVelocity.y * (deltaTime * 5);
		pitch = glm::clamp(pitch, -89.0f, 89.0f);

		yaw += cursorVelocity.x * (deltaTime * 5);
	}

	glm::vec3 cameraFacing;

	cameraFacing.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
	cameraFacing.y = sin(glm::radians(pitch));
	cameraFacing.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));

	cameraForward = glm::normalize(cameraFacing);

	cameraPosition += (zMovement * deltaTime) * cameraForward;
	cameraPosition -= (xMovement * deltaTime) * glm::cross(cameraForward, cameraUp);

	camMatrix = glm::lookAt(cameraPosition, cameraPosition + cameraForward, cameraUp);

	if (!orthographic)
		projMatrix = glm::perspective(glm::radians(fieldOfView), windowDimensions.x / windowDimensions.y, nearCull, farCull);
	else
		projMatrix = glm::ortho(0.0f, windowDimensions.x, 0.0f, windowDimensions.y);
}
