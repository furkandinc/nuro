#pragma once

#include <engine.h>
#include <rendering/gizmos/imgizmo.h>
#include <physics/core/physics_context.h>

#include <filesystem>
namespace fs = std::filesystem;

#include "../src/project/project.h"
#include "../src/pipelines/game_view_pipeline.h"
#include "../src/pipelines/scene_view_pipeline.h"
#include "../src/pipelines/preview_pipeline.h"

class ShadowDisk;
class ShadowMap;

class Model;
class LitMaterial;

enum class GameState {
	GAME_SLEEPING,
	GAME_LOADING,
	GAME_RUNNING,
	GAME_PAUSED,
};

namespace Runtime
{
	//
	// Base functions
	//

	int START_LOOP();
	int TERMINATE();

	//
	// Project
	//

	void loadProject(const fs::path& path);
	const Project& getProject();

	//
	// Game functions
	//

	void startGame();
	void stopGame();
	void pauseGame();
	void continueGame();

	GameState getGameState();

	//
	// Pipelines getters
	//

	SceneViewPipeline& getSceneViewPipeline();
	GameViewPipeline& getGameViewPipeline();
	PreviewPipeline& getPreviewPipeline();

	//
	// Physics getters
	//

	PhysicsContext& getGamePhysics();

	//
	// Gizmo getters
	//

	IMGizmo& getSceneGizmos();

	//
	// Shadow getters
	//

	ShadowDisk* getMainShadowDisk();
	ShadowMap* getMainShadowMap();
};