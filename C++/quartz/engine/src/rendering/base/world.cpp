#include "world.hpp"

#include "light.hpp"

void World::Update() 
{
	for (auto light : lights)
		light->Update();
}