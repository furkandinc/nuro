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
};