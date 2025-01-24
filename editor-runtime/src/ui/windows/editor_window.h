#pragma once

#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui.h>
#include <glm.hpp>

#include "../core/utils/log.h"
#include "../core/time/time.h"
#include "../src/ui/ui_utils.h"
#include "../src/ui/ui_flex.h"
#include "../src/ui/editor_ui.h"
#include "../src/ui/context_menu.h"
#include "../src/runtime/runtime.h"
#include "../src/ui/IconsFontAwesome6.h"
#include "../src/ui/inspectables/inspectable.h"
#include "../src/ui/components/im_components.h"
#include "../src/ui/dynamic_drawing/dynamic_drawing.h"

class EditorWindow
{
public:
	virtual void render() {};
};
