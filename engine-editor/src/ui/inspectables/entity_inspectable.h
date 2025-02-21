#pragma once

#include <functional>

#include "inspectable.h"

#include "../src/core/ecs/ecs_collection.h"
#include "../src/ui/windows/registry_window.h"

class EntityInspectable : public Inspectable
{
public:
	EntityInspectable(HierarchyItem& item);

	void renderStaticContent(ImDrawList& drawList) override;
	void renderDynamicContent(ImDrawList& drawList) override;
private:
	// Currently inspected hierarchy item
	HierarchyItem& item;

	// Adds a component to be drawn if its owned by the entity of the currently inspected hierarchy item
	template <typename T>
	void component(std::function<void(Entity entity, T&)> drawFunction) {
		if (item.entity.has<T>()) {
			T& component = item.entity.get<T>();
			drawFunction(item.entity.root, component);
		}
	}
};