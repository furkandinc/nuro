#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <vector>
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <map>

#include <stb_image.h>

#include "../engine/rendering/core/render_core.h"

#include "../engine/rendering/shader/shader.h"
#include "../engine/rendering/shader/shader_builder.h"

#include "../engine/rendering/texture/texture.h"

#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>
#include <gtc/quaternion.hpp>

#include "../engine/engineui/engine_ui.h"
#include "../engine/engineui/engine_dialog.h"

#include <json/json.hpp>
using json = nlohmann::json;

#include "../engine/utils/log.h"

#include "../engine/objects/entity.h"
#include "../engine/objects/camera.h"

#include "../user/src/game_logic.h"

#include "../engine/context/context.h"

#include "../engine/input/input.h"

#include "../engine/utils/vector_helper.h"

#include "../engine/utils/inspector_mode.h"

class Runtime {
public:
	// Entity system
	static std::vector<Entity*> entityLinks;
	static Entity* createEntity();

	// Camera settings
	static Camera* activeCamera;
	static Camera* inspectorCamera;
	static void useCamera(Camera* camera);

	// Time
	static float time;
	static float last_time;
	static float delta_time;

	// Render settings
	static bool wireframe;

	// Engine settings
	static bool inspectorMode;
	static bool showEngineUI;
};