#pragma once

#include <glm/glm.hpp>

namespace Engine
{
	struct Bounds {
	public:
		glm::vec3 from;
		glm::vec3 to;

		bool Contains(const glm::vec3 &position);
	};
}