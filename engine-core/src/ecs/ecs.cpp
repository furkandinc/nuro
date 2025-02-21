#include <ecs/ecs.h>

#include <random>
#include <glm/glm.hpp>
#include <algorithm>

#include <utils/console.h>
#include <ecs/reflection.h>
#include <transform/transform.h>

namespace ECS {

	RenderQueue gRenderQueue;

	void setup() {
		Reflection::setup();
	}

	std::tuple<Entity, TransformComponent&> createEntity(TransformComponent* parent)
	{
		// Create new entity and emplace transform component
		Entity entity = gRegistry.create();
		TransformComponent& transform = gRegistry.emplace<TransformComponent>(entity);

		// Add parent if set
		if (parent) transform.parent = parent;

		// Perform initial transform evaluation with empty perspective matrix
		Transform::evaluate(transform, glm::perspective(0.0f, 0.0f, 0.0f, 0.0f));

		// Return entity and transform component
		return std::tuple<Entity, TransformComponent&>(entity, transform);
	}

	RenderQueue& getRenderQueue()
	{
		return gRenderQueue;
	}

	void generateRenderQueue()
	{
		// Fill target queue
		std::vector<Entity> targetQueue;

		gRegistry.view<MeshRendererComponent>().each([&](auto entity, const auto&) { 
			targetQueue.push_back(entity); 
		});

		// Sort targets by shader and material
		std::sort(targetQueue.begin(), targetQueue.end(), [&](auto lhsEntity, auto rhsEntity) {
			MeshRendererComponent& lhs = gRegistry.get<MeshRendererComponent>(lhsEntity);
			auto lhsShaderId = lhs.material ? lhs.material->getShaderId() : -1;
			auto lhsMaterialId = lhs.material ? lhs.material->getId() : -1;

			MeshRendererComponent& rhs = gRegistry.get<MeshRendererComponent>(rhsEntity);
			auto rhsShaderId = rhs.material ? rhs.material->getShaderId() : -1;
			auto rhsMaterialId = rhs.material ? rhs.material->getId() : -1;

			return std::tie(lhsShaderId, lhsMaterialId) < std::tie(rhsShaderId, rhsMaterialId);
		});

		// Fill render queue
		gRenderQueue.clear();

		for (auto entity : targetQueue) {
			gRenderQueue.emplace_back(entity, gRegistry.get<TransformComponent>(entity), gRegistry.get<MeshRendererComponent>(entity));
		}
	}

	std::optional<Camera> getLatestCamera() {
		auto group = gRegistry.group<TransformComponent>(entt::get<CameraComponent>);
		for (auto entity : group) {
			auto [transform, camera] = group.get<TransformComponent, CameraComponent>(entity);

			if (camera.enabled) return Camera(transform, camera);
			else return std::nullopt;
		}
		return std::nullopt;
	}

}