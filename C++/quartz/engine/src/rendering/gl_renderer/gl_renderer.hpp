#pragma once

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "glsl/glsl_shader.hpp"

#include <debug/common.hpp>

#include "../base/renderer.hpp"

typedef void(*GLKeyDelegate)(GLFWwindow* window, int, int);

class GLRenderer : public Renderer
{

public:
	GLRenderer();

	void Update();
	void Terminate();

	std::vector<GLKeyDelegate> keyDelegates;

	DebugMenu* debugMenu;

	bool minimized = false;
	bool focused = true;

	bool shouldShutdown = false;

	static GLRenderer* instance;

private:
	GLFWwindow* gl_window;

	glm::vec2 lastCursorPos;
	glm::vec2 cursorVelocity;

};

