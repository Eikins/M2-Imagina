#include <vector>
#include <map>
#include <cmath>
#include <array>
#include <utility>


// Include GLEW
#include <GL/glew.h>

#include "Mesh.h"
#include "Octree.h"

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

void Engine::Mesh::ComputeNormals(const NormalWeightType weightType)
{
	uint32_t vertexCount = _vertices.size();
	uint32_t indexCount = _indices.size();

	_normals.clear();
	_normals.resize(vertexCount, glm::vec3(0.0));

	switch (weightType)
	{
		case NormalWeightType::Uniforms:
		{
			for (int i = 0; i < indexCount; i += 3)
			{
				glm::vec3 v0 = _vertices[_indices[i + 0]];
				glm::vec3 v1 = _vertices[_indices[i + 1]];
				glm::vec3 v2 = _vertices[_indices[i + 2]];

				glm::vec3 normal = glm::normalize(glm::cross(v1 - v0, v2 - v0));
				_normals[_indices[i + 0]] += normal;
				_normals[_indices[i + 1]] += normal;
				_normals[_indices[i + 2]] += normal;
			}
			break;
		}
		case NormalWeightType::PerArea:
		{
			for (int i = 0; i < indexCount; i += 3)
			{
				glm::vec3 v0 = _vertices[_indices[i + 0]];
				glm::vec3 v1 = _vertices[_indices[i + 1]];
				glm::vec3 v2 = _vertices[_indices[i + 2]];

				glm::vec3 normal = glm::cross(v1 - v0, v2 - v0);
				_normals[_indices[i + 0]] += normal;
				_normals[_indices[i + 1]] += normal;
				_normals[_indices[i + 2]] += normal;
			}
			break;
		}
		case NormalWeightType::PerAngle:
		{
			for (int i = 0; i < indexCount; i += 3)
			{
				glm::vec3 v0 = _vertices[_indices[i + 0]];
				glm::vec3 v1 = _vertices[_indices[i + 1]];
				glm::vec3 v2 = _vertices[_indices[i + 2]];

				float a[3]{
					glm::acos(glm::dot(normalize(v1 - v0), normalize(v2 - v0))),
					glm::acos(glm::dot(normalize(v2 - v1), normalize(v0 - v1))),
					glm::acos(glm::dot(normalize(v0 - v2), normalize(v1 - v2))),
				};

				glm::vec3 normal = glm::normalize(glm::cross(v1 - v0, v2 - v0));
				_normals[_indices[i + 0]] += a[0] * normal;
				_normals[_indices[i + 1]] += a[1] * normal;
				_normals[_indices[i + 2]] += a[2] * normal;
			}
			break;
		}
	}

	for (int i = 0; i < vertexCount; i++)
	{
		_normals[i] = glm::normalize(_normals[i]);
	}

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
			indexMap[gridPos] = vertexSums.size() - 1;
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

void Engine::Mesh::AdaptiveSimplify(uint32_t maxClusterVertexCount)
{
	Engine::Utils::Octree octree(GetBounds(0.05F), maxClusterVertexCount);
	int vertexCount = _vertices.size();

	for (int i = 0; i < vertexCount; i++)
	{
		Engine::Utils::OctreeItem item{};
		item.position = _vertices[i];
		item.normal = _vertices[i];
		item.uv = _texCoords[i];
		item.index = i;
		octree.Insert(_vertices[i], item);
	}

	// Compute centroids
	std::vector<unsigned> itemIndices(vertexCount);
	std::vector<Engine::Utils::OctreeItem> centroids;

	octree.ApplyOnLeaves([&itemIndices, &centroids](std::vector<Engine::Utils::OctreeItem>& items) {
		Engine::Utils::OctreeItem centroid;
		centroid.position = glm::vec3(0);
		centroid.normal = glm::vec3(0);
		centroid.uv = glm::vec2(0);
		unsigned itemCount = items.size();
		centroid.index = centroids.size();
		for (int i = 0; i < items.size(); i++)
		{
			centroid.position += items[i].position;
			centroid.normal += items[i].normal;
			centroid.uv += items[i].uv;
			itemIndices[items[i].index] = centroid.index;
		}

		centroid.position /= itemCount;
		centroid.normal = glm::normalize(centroid.normal);
		centroid.uv /= itemCount;

		centroids.push_back(centroid);
	});

	// Triangle remap
	std::vector<unsigned short> newTriangles;
	for (int i = 0; i < _indices.size(); i += 3)
	{
		unsigned i0 = itemIndices[_indices[i + 0]];
		unsigned i1 = itemIndices[_indices[i + 1]];
		unsigned i2 = itemIndices[_indices[i + 2]];

		if (i0 != i1 && i0 != i2 && i1 != i2)
		{
			newTriangles.push_back(i0);
			newTriangles.push_back(i1);
			newTriangles.push_back(i2);
		}
	}

	// Update mesh
	unsigned centroidCount = centroids.size();
	_vertices.resize(centroidCount);
	_normals.resize(centroidCount);
	_texCoords.resize(centroidCount);

	for (int i = 0; i < centroidCount; i++)
	{
		_vertices[i] = centroids[i].position;
		_normals[i] = centroids[i].normal;
		_texCoords[i] = centroids[i].uv;
	}

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
