#pragma once
#include <glm/glm.hpp>

class GizmoBillboard;
class Texture;

struct Light
{
	enum LightType 
	{
		Sun, Point
	};

	Light(LightType type);
	void Update();

	float strength = 0.5f;
	float range = 0.5f;
	glm::vec3 color = glm::vec3(1, 1, 1);
	glm::vec3 position = glm::vec3(0, 0, 0);
	bool enabled = true;

	LightType type = LightType::Point;

	const char* GetTypeName() 
	{
		if (type > 1 || type < 0)
			return "Unknown";

		switch (type) 
		{
		case LightType::Point:
			return "Point";

		case LightType::Sun:
			return "Sun";
		}
	}

	Texture* pointIcon;
	Texture* sunIcon;

	Texture* GetTypeIcon() 
	{
		if (type > 1 || type < 0)
			return pointIcon;

		switch (type)
		{
		case LightType::Point:
			return pointIcon;

		case LightType::Sun:
			return sunIcon;
		}
	}

private:
	GizmoBillboard* gizmo;

};

