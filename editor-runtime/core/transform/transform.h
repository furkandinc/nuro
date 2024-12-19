#pragma once

#include <glm.hpp>
#include <gtc/quaternion.hpp>

#include "../core/ecs/components.h"

namespace Transform
{

	// Transform related direction vectors
	glm::vec3 forward(TransformComponent& transform);
	glm::vec3 backward(TransformComponent& transform);
	glm::vec3 right(TransformComponent& transform);
	glm::vec3 left(TransformComponent& transform);
	glm::vec3 up(TransformComponent& transform);
	glm::vec3 down(TransformComponent& transform);

};