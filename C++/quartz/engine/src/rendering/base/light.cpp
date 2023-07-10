#include "light.hpp"

#include <rendering/gl_renderer/gl_model.hpp>
#include <debug/gizmos/gizmobillboard.hpp>
#include <filesystem/agnosticfs.hpp>

#include <rendering/base/light.hpp>

#include <rendering/base/material.hpp>
#include <rendering/base/texture.hpp>

#include <iostream>

Light::Light(LightType type) 
{
	this->type = type;

	gizmo = new GizmoBillboard(&position);

	AgnosticFile pointIconFile = AgnosticFileSystem::FindFileRelative("Data/Textures/Gizmos/Light_Point.png");
	AgnosticFile sunIconFile = AgnosticFileSystem::FindFileRelative("Data/Textures/Gizmos/Light_Sun.png");

	sunIcon = Texture::Load(&sunIconFile);
	pointIcon = Texture::Load(&pointIconFile);

	gizmo->material->tex0 = pointIcon;
}

void Light::Update() 
{
	gizmo->material->tex0 = GetTypeIcon();

	gizmo->material->albedoColor = glm::vec4(color * strength, 1);
	gizmo->visible = enabled;

	if (type == LightType::Sun)
		GizmoBillboard::DrawLine(glm::vec3(0, 0, 0), position);
}