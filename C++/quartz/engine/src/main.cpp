#include "common.hpp"

#include <filesystem/datamanager.hpp>
#include <rendering/gl_renderer/gl_renderer.hpp>

#include <debug/error_window.hpp>

using namespace std;

int main(int argc, char** argv)
{
	DataManager::Initialize();
	Renderer* instance = new GLRenderer();

	try 
	{
		while (instance != nullptr)
		{
			if (!instance->active)
				break;

			instance->Update();
		}
	}
	catch (std::exception except) 
	{
		ErrorWindow::Display(except);
	}
}