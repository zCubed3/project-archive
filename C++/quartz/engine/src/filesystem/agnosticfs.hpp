#pragma once

#include <common.hpp>

#ifdef WINDOWS
#include <Windows.h>
#endif

#include <string>
#include <vector>

#include <iostream>

struct AgnosticDirectory 
{
	AgnosticDirectory(std::string name, std::string path)
	{
		dirName = name;
	}

	std::string dirName;
	std::string dirPath;

	std::vector<AgnosticDirectory> subDirectories;
};

struct AgnosticFile 
{
	AgnosticFile(std::string name, std::string extension, std::string path) 
	{
		fileName = name;
		fileExtension = extension;
		filePath = path;
	}

	std::string fileName;
	std::string fileExtension;
	std::string filePath;
};

class AgnosticFileSystem
{

public:
	static std::string GetExecutionPath()
	{
		std::string returner;

#ifdef WINDOWS

		char temp[MAX_PATH];
		GetModuleFileNameA(nullptr, temp, MAX_PATH);

		returner = temp;
		BackToForward(&returner);
		TrimToSlash(&returner);

		return returner;

#endif

		throw std::exception("Execution Path Getter not implemented for platform");
	};

	static std::string GetExecutableLocation()
	{
		std::string returner;

#ifdef WINDOWS

		char temp[MAX_PATH];
		GetModuleFileNameA(nullptr, temp, MAX_PATH);

		return std::string(temp);

#endif

		throw std::exception("Execution Path Getter not implemented for platform");
	};

	static void BackToForward(std::string* convertee)
	{
		for (int c = 0; c < convertee->length(); c++)
			if (convertee->at(c) == '\\')
				convertee->at(c) = '/';
	};

	static void TrimToSlash(std::string* trimee)
	{
		while (trimee->at(trimee->length() - 1) != '/')
			trimee->pop_back();
	};

	static void TrimToFileName(std::string* trimee)
	{
		size_t slashIndex = trimee->find_last_of('/');

		if (slashIndex != std::string::npos)
			trimee->erase(trimee->begin(), trimee->begin() + ++slashIndex);

		size_t periodIndex = trimee->find_last_of('.');

		if (periodIndex != std::string::npos)
			trimee->erase(trimee->begin() + periodIndex, trimee->end());
	}

	static void TrimToFileExtension(std::string* trimee)
	{
		size_t periodIndex = trimee->find_last_of('.');

		if (periodIndex != std::string::npos)
			trimee->erase(trimee->begin(), trimee->begin() + periodIndex);
	}

	static void GetFiles(const char* path, std::vector<AgnosticFile>* files)
	{

#ifdef WINDOWS
		WIN32_FIND_DATA fData = { };

		HANDLE hFind = FindFirstFile((std::string(path) + "*").c_str(), &fData);

		if (hFind != INVALID_HANDLE_VALUE)
			if (IsValidFile(fData.cFileName) && !(fData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
			{
				std::string name(fData.cFileName), extension(fData.cFileName);

				TrimToFileExtension(&extension);
				TrimToFileName(&name);

				files->push_back(AgnosticFile(name, extension, (std::string(path) + fData.cFileName)));
			}

		while (FindNextFile(hFind, &fData)) 
		{
			if (IsValidFile(fData.cFileName) && !(fData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
			{
				std::string name(fData.cFileName), extension(fData.cFileName);

				TrimToFileExtension(&extension);
				TrimToFileName(&name);

				files->push_back(AgnosticFile(name, extension, (std::string(path) + fData.cFileName)));
			}
		}

		FindClose(hFind);
#endif

	};

	static void GetSubfolders(const char* path, std::vector<AgnosticDirectory>* directories)
	{

#ifdef WINDOWS
		WIN32_FIND_DATA fData = { };

		HANDLE hFind = FindFirstFile((std::string(path) + "*").c_str(), &fData);

		if (hFind != INVALID_HANDLE_VALUE)
			if (fData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
				directories->push_back(AgnosticDirectory(std::string(path) + fData.cFileName, fData.cFileName));

		while (FindNextFile(hFind, &fData))
			if (fData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
				directories->push_back(AgnosticDirectory(std::string(path) + fData.cFileName, fData.cFileName));

		FindClose(hFind);
#endif

	};

	static bool FileExists(const char* path) 
	{
		
#ifdef WINDOWS
		
		DWORD fileAttributes = GetFileAttributes(path);
		return (fileAttributes != INVALID_FILE_ATTRIBUTES) && !(fileAttributes & FILE_ATTRIBUTE_DIRECTORY);

#endif

		return false;
	}

	static AgnosticFile FindFileRelative(const char* path)
	{
		std::string execPath = GetExecutionPath();

		execPath += path;

		std::string trimName(execPath), trimExtension(execPath);

		TrimToFileExtension(&trimExtension);
		TrimToFileName(&trimName);

#ifdef WINDOWS

		DWORD fileAttribs = GetFileAttributes(execPath.c_str());

		if (fileAttribs != INVALID_FILE_ATTRIBUTES && !(fileAttribs & FILE_ATTRIBUTE_DIRECTORY))
			return AgnosticFile(trimName, trimExtension, execPath);

#endif

		return AgnosticFile("", "", "");
	}

	static void CreateDir(const char* path) 
	{
		
#ifdef WINDOWS

		CreateDirectory(path, NULL);

#endif

	}

private:
	static bool IsValidFile(const char* path) { return (std::string(path) != "." && std::string(path) != ".."); };

};

