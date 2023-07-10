#include "datamanager.hpp"

#include <vector>
#include <filesystem/agnosticfs.hpp>

void DataManager::Initialize()
{
	execFolder = AgnosticFileSystem::GetExecutionPath();
	dataFolder = execFolder + "Data/";

	// Verify if each of the neccesary folders exist, if not, create them
	VerifyFolders();

	std::vector<AgnosticFile> files;
	AgnosticFileSystem::GetFiles(AgnosticFileSystem::GetExecutionPath().c_str(), &files);
}

std::string DataManager::GetDataSubfolder(DataSubdirType subdirType) 
{
	std::string path = dataFolder;

	switch (subdirType) 
	{
	default: 
		break;

	case DataSubdirType::Modules:
		path += "Modules";
		break;

	case DataSubdirType::Models:
		path += "Models";
		break;

	case DataSubdirType::Shaders:
		path += "Shaders";
		break;

	case DataSubdirType::Sounds:
		path += "Sounds";
		break;

	case DataSubdirType::Textures:
		path += "Textures";
		break;

	case DataSubdirType::Fonts:
		path += "Fonts";
		break;
	}

	path += "/";

	return path;
}

void DataManager::VerifyFolders() 
{
	std::vector<DataSubdirType> subdirTypes;

	subdirTypes.push_back(DataSubdirType::Models);
	subdirTypes.push_back(DataSubdirType::Modules);
	subdirTypes.push_back(DataSubdirType::Sounds);
	subdirTypes.push_back(DataSubdirType::Textures);
	subdirTypes.push_back(DataSubdirType::Shaders);
	subdirTypes.push_back(DataSubdirType::Fonts);

	AgnosticFileSystem::CreateDir(dataFolder.c_str());

	for (int sT = 0; sT < subdirTypes.size(); sT++) 
	{
		std::string path = GetDataSubfolder(subdirTypes[sT]);
		AgnosticFileSystem::CreateDir(path.c_str());
	}
}

// Static bullshite
std::string DataManager::execFolder = "";
std::string DataManager::dataFolder = "";