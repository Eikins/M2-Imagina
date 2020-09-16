#pragma once

#include <cstdlib>
#include <vector>

// Include GLM
#include <glm/glm.hpp>

namespace Engine {


	struct Bounds {
	public:
		glm::vec3 from;
		glm::vec3 to;
	};

	class Mesh {

	public:
		std::vector<glm::vec3> _vertices;
		std::vector<glm::vec3> _normals;
		std::vector<glm::vec2> _texCoords;

		std::vector<unsigned short> _indices;

		Mesh() {}

		Mesh(std::vector<glm::vec3> vertices, std::vector<glm::vec2> texCoords, std::vector<unsigned short> triangles) :
			_vertices(vertices),
			_normals(vertices.size()),
			_texCoords(texCoords),
			_indices(triangles) {}

		Mesh(std::vector<glm::vec3> vertices, std::vector<glm::vec3> normals, std::vector<glm::vec2> texCoords, std::vector<unsigned short> triangles) :
			_vertices(vertices),
			_normals(normals),
			_texCoords(texCoords),
			_indices(triangles) {}

		Bounds GetBounds(float offset = 0.0);

		void Simplify(unsigned resolution);

		void DrawBounds();

	};

}
