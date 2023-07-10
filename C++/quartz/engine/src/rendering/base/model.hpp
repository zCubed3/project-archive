#pragma once

#include <vector>
#include <map>

#include <sstream>
#include <iomanip>
#include <fstream>
#include <iostream>

#include <rendering/base/renderer.hpp>

#include <glm/glm.hpp>

class Material;
struct AgnosticFile;

class Model
{
public:
	enum Primitives
	{
		Quad, Triangle
	};

	template<typename ModelType>
	static Model* Load(AgnosticFile file)
	{
		for (auto const& [key, value] : Model::cachedModels)
			if (strcmp(key, file.fileName.c_str()) == 0) 
				return value;

		if (file.fileExtension == ".obj")
		{
			Model* allocatedModel = new ModelType();

			allocatedModel->name = file.fileName;

			std::ifstream sFile(file.filePath, std::ios::in);

			if (sFile.is_open())
			{
				std::string line;

				std::vector<glm::vec2> tUVs;
				std::vector<glm::vec3> tNormals;
				std::vector<glm::vec3> tVertices;
				std::vector<unsigned int> tIndices;

				std::vector<glm::ivec3> vertIndices;

				while (std::getline(sFile, line))
				{
					std::string lineID = line.substr(0, line.find_first_of(' '));
					std::string rawData = line.substr(line.find_first_of(' '));
					std::stringstream lineStream(rawData);

					if (lineID == "v")
					{
						float x{}, y{}, z{};
						lineStream >> x >> y >> z;

						tVertices.push_back(glm::vec3(x, y, z));
					}

					if (lineID == "vt")
					{
						float u{}, v{};
						lineStream >> u >> v;

						tUVs.push_back(glm::vec2(u, v));
					}

					if (lineID == "vn")
					{
						float x{}, y{}, z{};
						lineStream >> x >> y >> z;

						tNormals.push_back(glm::vec3(x, y, z));
					}

					if (lineID == "f")
					{
						lineStream.ignore(line.length(), '/');
						unsigned int vIndex[3]{ 0,0,0 }, uvIndex[3]{ 0,0,0 }, nIndex[3]{ 0,0,0 };

						sscanf_s(rawData.c_str(), "%u/%u/%u %u/%u/%u %u/%u/%u\n",
							&vIndex[0], &uvIndex[0], &nIndex[0],
							&vIndex[1], &uvIndex[1], &nIndex[1],
							&vIndex[2], &uvIndex[2], &nIndex[2]
						);

						vertIndices.push_back(glm::vec<3, int>(--vIndex[0], --uvIndex[0], --nIndex[0]));
						vertIndices.push_back(glm::vec<3, int>(--vIndex[1], --uvIndex[1], --nIndex[1]));
						vertIndices.push_back(glm::vec<3, int>(--vIndex[2], --uvIndex[2], --nIndex[2]));

						tIndices.push_back(vIndex[0]);
						tIndices.push_back(vIndex[1]);
						tIndices.push_back(vIndex[2]);
					}
				}

				std::cout << vertIndices.size() << "\n";

				for (int v = 0; v < vertIndices.size(); v++)
				{
					allocatedModel->indices.push_back(allocatedModel->vertices.size());

					allocatedModel->vertices.push_back(tVertices[vertIndices[v].x]);
				
					allocatedModel->uvs.push_back(tUVs[vertIndices[v].y]);
				
					allocatedModel->normals.push_back(tNormals[vertIndices[v].z]);
				}

				sFile.close();

				allocatedModel->Finalize();
				Model::cachedModels.insert(std::make_pair(allocatedModel->name.c_str(), allocatedModel));

				LOG_RENDERER << "Cached " << allocatedModel->name << "\n";

				return allocatedModel;
			}
			else
				throw std::exception("File does not exist!");
		}

		return nullptr;
	};

	template<typename ModelType>
	static Model* Load(Primitives prim)
	{
		switch (prim)
		{
		default:
		case Primitives::Quad:
		{
			if (Model::cachedModels.find("Quad") != Model::cachedModels.end())
				return Model::cachedModels.at("Quad");

			Model* allocatedModel = new ModelType();

			allocatedModel->vertices = std::vector<glm::vec3>
			{
				glm::vec3(-1, -1, 0),
				glm::vec3(-1, 1, 0),
				glm::vec3(1, -1, 0),
				glm::vec3(1, 1, 0)
			};

			allocatedModel->normals = std::vector<glm::vec3>
			{
				glm::vec3(0, 0, -1),
				glm::vec3(0, 0, -1),
				glm::vec3(0, 0, -1),
				glm::vec3(0, 0, -1)
			};

			allocatedModel->uvs = std::vector<glm::vec2>
			{
				glm::vec2(0, 0),
				glm::vec2(0, 1),
				glm::vec2(1, 0),
				glm::vec2(1, 1)
			};

			allocatedModel->indices = std::vector<unsigned int>
			{
				0u,
				1u,
				2u,
				1u,
				3u,
				2u
			};

			allocatedModel->name = "Quad";
			allocatedModel->Finalize();

			Model::cachedModels.insert(std::make_pair(allocatedModel->name.c_str(), allocatedModel));

			return allocatedModel;
		} break;
		}

		return nullptr;
	};
	
	virtual void Finalize() = 0;
	virtual ~Model() {};

	virtual void Draw(Renderer* renderer, Material* material, glm::mat4 modelMatrix) = 0;

	std::vector<glm::vec3> vertices;
	std::vector<glm::vec2> uvs;
	std::vector<glm::vec3> normals;

	std::vector<unsigned int> indices;

	std::string name;

	static Model* primQuad;
	static Model* primTriangle;

	static void InitializePrimitives();

private:
	static std::map<const char*, Model*> cachedModels;

};

