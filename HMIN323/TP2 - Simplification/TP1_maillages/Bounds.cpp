#include "include\Bounds.h"

bool Engine::Bounds::Contains(const glm::vec3& position)
{
	return position.x > from.x && position.y > from.y && position.z > from.z &&
		   position.x < to.x && position.y < to.y && position.z < to.z;
}
