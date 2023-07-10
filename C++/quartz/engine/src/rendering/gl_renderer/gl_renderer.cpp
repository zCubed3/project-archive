#include <common.hpp>

#include "gl_renderer.hpp"
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <imgui/imgui.h>
#include <imgui/backends/imgui_impl_glfw.h>
#include <imgui/backends/imgui_impl_opengl3.h>

#include "gl_model.hpp"

#include "../base/camera.hpp"

#include <entities/entity.hpp>

#include <debug/gizmos/gizmobillboard.hpp>

#include <filesystem/datamanager.hpp>
#include <filesystem/agnosticfs.hpp>

#include <threading/agnosticthread.hpp>

#include "../base/material.hpp"
#include "../base/light.hpp"

GLRenderer* GLRenderer::instance = nullptr;

using namespace ImGui;

// Callbacks
void OnFocused(GLFWwindow* window, int focused)
{
	GLRenderer::instance->focused = focused;
}

void OnMinimized(GLFWwindow* window, int minimized) 
{
	GLRenderer::instance->minimized = minimized;
}

void OnFramebufferResized(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
}

void OnKeyInput(GLFWwindow* window, int key, int scancode, int action, int mods) 
{
	for (int d = 0; d < GLRenderer::instance->keyDelegates.size(); d++)
	{
		if (GLRenderer::instance->keyDelegates[d] == nullptr)
			GLRenderer::instance->keyDelegates.erase(GLRenderer::instance->keyDelegates.begin() + d);
		else
			GLRenderer::instance->keyDelegates[d](window, key, action);
	}
}

// Ctor initializes the default shaders
GLRenderer::GLRenderer() 
{
	LOG_RENDERER << "Initializing OpenGL\n";

	// Initialize OpenGL (GLFW and GLEW)
	if (glfwInit() == GLFW_TRUE)
	{
		// Limit the API to 3.3+
		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);

		// Then do the rest
		gl_window = glfwCreateWindow(DEFAULT_WIN_WIDTH, DEFAULT_WIN_HEIGHT, GAME_NAME, nullptr, nullptr);
		glfwMakeContextCurrent(gl_window);

		// Setup some basic callbacks
		glfwSetFramebufferSizeCallback(gl_window, OnFramebufferResized);
		glfwSetKeyCallback(gl_window, OnKeyInput);
		glfwSetWindowIconifyCallback(gl_window, OnMinimized);
		glfwSetWindowFocusCallback(gl_window, OnFocused);

		// Setup ImGui
		ImGui::CreateContext();
		ImGui_ImplGlfw_InitForOpenGL(gl_window, true);
		ImGui_ImplOpenGL3_Init("#version 330");

		std::string fontPath = (DataManager::GetDataSubfolder(DataManager::Fonts) + "EditorFont.ttf").c_str();
		ImFont* font = nullptr;

		if (AgnosticFileSystem::FileExists(fontPath.c_str())) 
			font = ImGui::GetIO().Fonts->AddFontFromFileTTF(fontPath.c_str(), 16.0f);

		if (font != nullptr)
			ImGui::GetIO().FontDefault = font;

		ImGuiStyle& style = ImGui::GetStyle();

		ImVec4 activeColor = RGB_TO_LINEAR_VEC4(ImVec4, 175, 0, 82, 255);
		ImVec4 inactiveColor = RGB_TO_LINEAR_VEC4(ImVec4, 104, 0, 48, 255);
		ImVec4 hoverColor = RGB_TO_LINEAR_VEC4(ImVec4, 52, 0, 24, 255);

		style.Colors[ImGuiCol_TitleBgActive] = activeColor;
		style.Colors[ImGuiCol_TitleBg] = inactiveColor;
		style.Colors[ImGuiCol_TitleBgCollapsed] = hoverColor;

		style.Colors[ImGuiCol_ButtonActive] = activeColor;
		style.Colors[ImGuiCol_Button] = inactiveColor;
		style.Colors[ImGuiCol_ButtonHovered] = hoverColor;

		style.Colors[ImGuiCol_ResizeGripActive] = activeColor;
		style.Colors[ImGuiCol_ResizeGrip] = inactiveColor;
		style.Colors[ImGuiCol_ResizeGripHovered] = hoverColor;

		style.AntiAliasedLines = true;
		style.AntiAliasedLinesUseTex = true;
		style.AntiAliasedFill = true;

		if (gl_window == nullptr)
			throw std::exception("Failed to create an OpenGL Window!");

		GLenum initStatus = glewInit();
		glewGetErrorString(initStatus);

		active = true;

		glfwSwapInterval(1);

		// Enable OpenGL extensions
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LESS);
		glEnable(GL_CULL_FACE);
		glEnable(GL_TEXTURE_2D);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		// Initialize default / debug stuff
		debugMenu = new DebugMenu();

		GLSLShader::InitializeDefaults();
		Model::InitializePrimitives();

		// Initialize the world
		Light* sunLight = new Light(Light::LightType::Sun);
		world.lights.push_back(sunLight);

		sunLight->enabled = true;
		sunLight->position = glm::vec3(1, 1, 1);
		sunLight->strength = 1;
		sunLight->color = glm::vec3(1, 1, 1);

		instance = this;
		world.camera = new Camera(instance);


	}
	else
		throw std::exception("Failed to initialize OpenGL!");

}

void GLRenderer::Update() 
{ 
	if (gl_window == nullptr)
		return;

	// Slow window operations in the event the user is no longer focusing on it
	if (!focused)
		AgnosticThread::ThreadSleep(50);	

	// and finally don't render at all if the user's window is minimized
	if (minimized)
		return;

	// Get the cursor velocity
	double xpos, ypos;
	glfwGetCursorPos(gl_window, &xpos, &ypos);
	glm::vec2 cursorPos = glm::vec2(xpos, ypos);
	cursorVelocity = cursorPos - lastCursorPos;
	lastCursorPos = glm::vec2(xpos, ypos);

	// Timing
	deltaTime = glfwGetTime() - accumulatedTime;
	accumulatedTime += deltaTime;

	// Draw the menus
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();

	if (debugMenu != nullptr)
		debugMenu->Draw(instance);

	glClearColor(instance->world.clearColor.r, instance->world.clearColor.g, instance->world.clearColor.b, instance->world.clearColor.a);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	// Get the window width and height
	int width, height;
	glfwGetWindowSize(gl_window, &width, &height);

	// Update the camera
	if (world.camera != nullptr) 
	{
		world.camera->Update(deltaTime, cursorVelocity, glm::vec2((float)width, (float)height));
		viewMatrix = world.camera->camMatrix;
		projectionMatrix = world.camera->projMatrix;
	}
	else
		projectionMatrix = glm::perspective(glm::radians(70.0f), (float)width / (float)height, 0.1f, 100.0f);

	// Update the world
	world.Update();

	// Iterate through each entity and draw it
	for (int e = 0; e < world.entities.size(); e++)
		world.entities[e]->Draw(this);

	for (int g = 0; g < GizmoBillboard::billboards.size(); g++)
		GizmoBillboard::billboards[g]->Draw(this);

	// Draw the available UIs
	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

	glfwSwapBuffers(gl_window);
	glfwPollEvents();

	if (glfwWindowShouldClose(gl_window) || shouldShutdown) 
	{
		LOG_RENDERER << "Window closed, shutting down OpenGL.\n";
		Terminate();
	}
}

void GLRenderer::Terminate() 
{
	glfwDestroyWindow(gl_window);

	gl_window = nullptr;
	active = false;

	glfwTerminate();
}