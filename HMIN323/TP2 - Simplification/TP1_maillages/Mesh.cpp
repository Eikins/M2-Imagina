#include <vector>
#include <map>
#include <cmath>
#include <array>
#include <utility>


// Include GLEW
#include <GL/glew.h>

#include "Mesh.h"

Engine::Bounds Engine::Mesh::GetBounds(float offset)
{
	Bounds bounds;

	glm::vec3 from = _vertices[0];
	glm::vec3 to = _vertices[0];

	for (int i = 1; i < _vertices.size(); i++)
	{
		glm::vec3 &vertex = _vertices[i];

		if (vertex.x < from.x)
		{
			from.x = vertex.x;
		}

		if (vertex.y < from.y)
		{
			from.y = vertex.y;
		}

		if (vertex.z < from.z)
		{
			from.z = vertex.z;
		}

		if (vertex.x > to.x)
		{
			to.x = vertex.x;
		}

		if (vertex.y > to.y)
		{
			to.y = vertex.y;
		}

		if (vertex.z > to.z)
		{
			to.z = vertex.z;
		}
	}

	from.x -= offset;
	from.y -= offset;
	from.z -= offset;

	to.x -= offset;
	to.y -= offset;
	to.z -= offset;

	bounds.from = from;
	bounds.to = to;

	return bounds;
}

void Engine::Mesh::Simplify(unsigned resolution)
{
	Engine::Bounds bounds = GetBounds(0.01f);
	unsigned segments = resolution + 1;
	std::map<std::array<unsigned, 3>, unsigned> indexMap;

	std::vector<glm::vec3> vertexSums;
	std::vector<glm::vec3> normalSums;
	std::vector<glm::vec2> texCoordSums;
	std::vector<unsigned> counts;
	std::vector<unsigned short> newTriangles;

	glm::vec3 dxyz = (bounds.to - bounds.from) / (float) segments;

	for (int i = 0; i < _vertices.size(); i++)
	{
		glm::vec3 &vertex = _vertices[i];
		glm::vec3 gridPosf = (vertex - bounds.from) / dxyz;
		std::array<unsigned, 3> gridPos = { 
			(unsigned)std::floor(gridPosf.x),
			(unsigned)std::floor(gridPosf.y),
			(unsigned)std::floor(gridPosf.z)
		};
		
		auto it = indexMap.find(gridPos);
		if (it == indexMap.end())
		{
			vertexSums.push_back(vertex);
			normalSums.push_back(_normals[i]);
			texCoordSums.push_back(_texCoords[i]);
			counts.push_back(1);
			indexMap[gridPos] = vertexSums.size();
		}
		else
		{
			unsigned index = it->second;
			vertexSums[index] += vertex;
			normalSums[index] += _normals[i];
			texCoordSums[index] += _texCoords[i];
			counts[index]++;
		}
	}

	for (int i = 0; i < vertexSums.size(); i++)
	{
		vertexSums[i] /= counts[i];
		texCoordSums[i] /= counts[i];
		normalSums[i] = glm::normalize(normalSums[i]);
	}

	for (int i = 0; i < _indices.size(); i += 3)
	{
		std::array<int, 3> gridIndexes;
		for (int di = 0; di < 3; di++)
		{
			glm::vec3 &vertex = _vertices[_indices[i + di]];
			glm::vec3 gridPosf = (vertex - bounds.from) / dxyz;
			std::array<unsigned, 3> gridPos = {
				(unsigned)std::floor(gridPosf.x),
				(unsigned)std::floor(gridPosf.y),
				(unsigned)std::floor(gridPosf.z)
			};
			gridIndexes[di] = indexMap[gridPos];
		}

		if (gridIndexes[0] != gridIndexes[1] && gridIndexes[0] != gridIndexes[2] && gridIndexes[1] != gridIndexes[2])
		{
			newTriangles.push_back(gridIndexes[0]);
			newTriangles.push_back(gridIndexes[1]);
			newTriangles.push_back(gridIndexes[2]);
		}
	}

	_vertices = vertexSums;
	_normals = normalSums;
	_texCoords = texCoordSums;
	_indices = newTriangles;
}

void Engine::Mesh::DrawBounds()
{
	Bounds bounds = GetBounds(0.0f);
	glColor3f(0.0f, 1.0f, 0.0f);
	glBegin(GL_LINES);

		glVertex3f(0, 0, 0);
		glVertex3f(0, 20, 0);

		glVertex3f(bounds.from.x, bounds.from.y, bounds.from.z);
		glVertex3f(bounds.to.x, bounds.to.y, bounds.to.z);
	glEnd();
}
