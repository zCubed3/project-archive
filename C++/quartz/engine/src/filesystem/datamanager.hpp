#pragma once

#include <common.hpp>

#include <string>

class DataManager
{

public:
	enum DataSubdirType 
	{
		Models,
		Sounds,
		Shaders,
		Modules,
		Textures,
		Fonts
	};

	static void Initialize();
	static std::string GetDataSubfolder(DataSubdirType subdirType);

	static std::string execFolder;
	static std::string dataFolder;

private:
	static void VerifyFolders();

};

