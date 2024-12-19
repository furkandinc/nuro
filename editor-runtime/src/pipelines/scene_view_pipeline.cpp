#include "scene_view_pipeline.h"

#include "../core/input/input.h"
#include "../core/rendering/core/transformation.h"
#include "../core/rendering/core/mesh_renderer.h"
#include "../core/utils/log.h"
#include "../core/diagnostics/profiler.h"
#include "../core/rendering/material/lit/lit_material.h"
#include "../core/rendering/culling/bounding_volume.h"

#include "../src/ui/windows/scene_window.h"
#include "../src/runtime/runtime.h"

// initialize with users editor settings later
SceneViewPipeline::SceneViewPipeline() : wireframe(false),
useProfileEffects(false),
showSkybox(false),
showGizmos(false),
renderShadows(false),
viewport(),
msaaSamples(8), // should lower this
defaultProfile(),
flyCamera(),
prePass(viewport),
sceneViewForwardPass(viewport),
ssaoPass(viewport),
velocityBuffer(viewport),
postProcessingPipeline(viewport, false),
imGizmo()
{
}

void SceneViewPipeline::setup()
{
	// Setup quick gizmo
	imGizmo.setup();

	// Initialize default profile:

	// Neutral color settings
	defaultProfile.color.exposure = 1.0f;
	defaultProfile.color.contrast = 1.0f;
	defaultProfile.color.gamma = 2.2f;
	// All effects disabled
	defaultProfile.motionBlur.enabled = false;
	defaultProfile.bloom.enabled = false;
	defaultProfile.chromaticAberration.enabled = false;
	defaultProfile.vignette.enabled = false;
	defaultProfile.ambientOcclusion.enabled = false;

	// Create passes
	createPasses();
}

void SceneViewPipeline::render(std::vector<OldEntity*>& targets)
{
	Profiler::start("render");

	// Start new frame for quick gizmos
	imGizmo.newFrame();

	// Pick variable items for rendering
	std::pair<TransformComponent&, CameraComponent&> targetCamera(flyCamera.first, flyCamera.second);

	// Select game profile if profile effects are enabled and not rendering wireframe
	bool useDefaultProfile = !useProfileEffects || wireframe;
	PostProcessing::Profile& targetProfile = useDefaultProfile ? defaultProfile : Runtime::gameViewPipeline.getProfile();

	// Get transformation matrices
	glm::mat4 viewMatrix = Transformation::view(targetCamera.first.position, targetCamera.first.rotation);
	glm::mat4 projectionMatrix = Transformation::projection(targetCamera.second.fov, targetCamera.second.near, targetCamera.second.far, viewport);
	glm::mat4 viewProjectionMatrix = projectionMatrix * viewMatrix;
	glm::mat3 viewNormalMatrix = glm::transpose(glm::inverse(glm::mat3(viewMatrix)));

	// Set scene windows current matrix caches
	SceneWindow::viewMatrix = viewMatrix;
	SceneWindow::projectionMatrix = projectionMatrix;

	//
	// PREPARATION PASS
	// Prepare each mesh renderer for upcoming render passes
	//
	for (int i = 0; i < targets.size(); i++)
	{
		MeshRenderer& targetRenderer = targets[i]->meshRenderer;

		// Could be moved to existing iteration over entity links within some pass to avoid additional iteration overhead
		// Here for now to ensure preparation despite further pipeline changes
		targetRenderer.prepareNextFrame();

		if (showGizmos) {
			targetRenderer.volume->draw(imGizmo, glm::vec4(GizmoColor::GREEN, 0.5f));
		}
	}

	// Render light gizmos
	imGizmo.color = GizmoColor::BLUE;
	imGizmo.foreground = false;
	imGizmo.opacity = 0.08f;

	imGizmo.icon3d(GizmoIconPool::get("fa_lightbulb"), glm::vec3(0.0f, 0.0f, 6.5f), targetCamera.first, glm::vec3(0.35f));
	/*imGizmo.sphereWire(glm::vec3(0.0f, 0.0f, 6.5f), 1.0f);
	imGizmo.icon3d(GizmoIconPool::get("fa_lightbulb"), glm::vec3(12.0f, 1.9f, -4.0f), targetCamera.first, glm::vec3(0.35f));
	imGizmo.icon3d(GizmoIconPool::get("fa_lightbulb"), glm::vec3(4.0f, 0.0f, 6.5f), targetCamera.first, glm::vec3(0.35f));
	imGizmo.sphereWire(glm::vec3(4.0f, 0.0f, 6.5f), 1.0f);
	imGizmo.icon3d(GizmoIconPool::get("fa_lightbulb"), glm::vec3(8.0f, 0.0f, 6.5f), targetCamera.first, glm::vec3(0.35f));
	imGizmo.sphereWire(glm::vec3(8.0f, 0.0f, 6.5f), 1.0f);
	imGizmo.icon3d(GizmoIconPool::get("fa_lightbulb"), glm::vec3(12.0f, 0.0f, 6.5f), targetCamera.first, glm::vec3(0.35f));
	imGizmo.sphereWire(glm::vec3(12.0f, 0.0f, 6.5f), 1.0f);
	imGizmo.icon3d(GizmoIconPool::get("fa_lightbulb"), glm::vec3(16.0f, 0.0f, 6.5f), targetCamera.first, glm::vec3(0.35f));
	imGizmo.sphereWire(glm::vec3(16.0f, 0.0f, 6.5f), 1.0f);
	imGizmo.icon3d(GizmoIconPool::get("fa_lightbulb"), glm::vec3(20.0, 0.0f, 6.5f), targetCamera.first, glm::vec3(0.35f));
	imGizmo.sphereWire(glm::vec3(20.0f, 0.0f, 6.5f), 1.0f);
	imGizmo.icon3d(GizmoIconPool::get("fa_lightbulb"), glm::vec3(24.0f, 0.0f, 6.5f), targetCamera.first, glm::vec3(0.35f));
	imGizmo.sphereWire(glm::vec3(24.0f, 0.0f, 6.5f), 1.0f);
	imGizmo.icon3d(GizmoIconPool::get("fa_lightbulb"), glm::vec3(28.0f, 0.0f, 6.5f), targetCamera.first, glm::vec3(0.35f));
	imGizmo.sphereWire(glm::vec3(28.0f, 0.0f, 6.5f), 1.0f);
	imGizmo.icon3d(GizmoIconPool::get("fa_lightbulb"), glm::vec3(32.0f, 0.0f, 6.5f), targetCamera.first, glm::vec3(0.35f));
	imGizmo.sphereWire(glm::vec3(32.0f, 0.0f, 6.5f), 1.0f);
	imGizmo.icon3d(GizmoIconPool::get("fa_lightbulb"), glm::vec3(36.0f, 0.0f, 6.5f), targetCamera.first, glm::vec3(0.35f));
	imGizmo.sphereWire(glm::vec3(36.0f, 0.0f, 6.5f), 1.0f);
	imGizmo.icon3d(GizmoIconPool::get("fa_lightbulb"), glm::vec3(40.0f, 0.0f, 6.5f), targetCamera.first, glm::vec3(0.35f));
	imGizmo.sphereWire(glm::vec3(40.0f, 0.0f, 6.5f), 1.0f);
	imGizmo.icon3d(GizmoIconPool::get("fa_lightbulb"), glm::vec3(44.0f, 0.0f, 6.5f), targetCamera.first, glm::vec3(0.35f));
	imGizmo.sphereWire(glm::vec3(44.0f, 0.0f, 6.5f), 1.0f);
	imGizmo.icon3d(GizmoIconPool::get("fa_lightbulb"), glm::vec3(48.0f, 0.0f, 6.5f), targetCamera.first, glm::vec3(0.35f));
	imGizmo.sphereWire(glm::vec3(48.0f, 0.0f, 6.5f), 1.0f);
	imGizmo.icon3d(GizmoIconPool::get("fa_lightbulb"), glm::vec3(52.0f, 0.0f, 6.5f), targetCamera.first, glm::vec3(0.35f));
	imGizmo.sphereWire(glm::vec3(52.0f, 0.0f, 6.5f), 1.0f);
	imGizmo.icon3d(GizmoIconPool::get("fa_lightbulb"), glm::vec3(56.0f, 0.0f, 6.5f), targetCamera.first, glm::vec3(0.35f));
	imGizmo.sphereWire(glm::vec3(56.0f, 0.0f, 6.5f), 1.0f);*/

	//
	// PRE PASS
	// Create geometry pass with depth buffer before forward pass
	//
	Profiler::start("pre_pass");
	prePass.render(targets);
	Profiler::stop("pre_pass");
	const unsigned int PRE_PASS_DEPTH_OUTPUT = prePass.getDepthOutput();
	const unsigned int PRE_PASS_NORMAL_OUTPUT = prePass.getNormalOutput();

	//
	// SCREEN SPACE AMBIENT OCCLUSION PASS
	// Calculate screen space ambient occlusion if enabled
	//
	Profiler::start("ssao");
	unsigned int _ssaoOutput = 0;
	if (targetProfile.ambientOcclusion.enabled)
	{
		_ssaoOutput = ssaoPass.render(projectionMatrix, targetProfile, PRE_PASS_DEPTH_OUTPUT, PRE_PASS_NORMAL_OUTPUT);
	}
	const unsigned int SSAO_OUTPUT = _ssaoOutput;
	Profiler::stop("ssao");

	//
	// VELOCITY BUFFER RENDER PASS
	//
	Profiler::start("velocity_buffer");
	const unsigned int VELOCITY_BUFFER_OUTPUT = velocityBuffer.render(targetProfile, targets);
	Profiler::stop("velocity_buffer");

	//
	// FORWARD PASS: Perform rendering for every object with materials, lighting etc.
	// Includes injected pre pass
	//

	// Prepare lit material with current render data
	LitMaterial::viewport = &viewport; // Redundant most of the times atm
	LitMaterial::cameraTransform = &targetCamera.first; // Redundant most of the times atm
	LitMaterial::ssaoInput = SSAO_OUTPUT;
	LitMaterial::profile = &targetProfile;
	LitMaterial::castShadows = renderShadows;
	LitMaterial::mainShadowDisk = Runtime::mainShadowDisk;
	LitMaterial::mainShadowMap = Runtime::mainShadowMap;

	Profiler::start("forward_pass");
	sceneViewForwardPass.wireframe = wireframe;
	sceneViewForwardPass.drawSkybox = showSkybox;
	sceneViewForwardPass.setSkybox(Runtime::gameViewPipeline.getSkybox());
	sceneViewForwardPass.drawQuickGizmos = showGizmos;
	unsigned int FORWARD_PASS_OUTPUT = sceneViewForwardPass.render(targets, nullptr, viewMatrix, projectionMatrix, viewProjectionMatrix);
	Profiler::stop("forward_pass");

	//
	// POST PROCESSING PASS
	// Render post processing pass to screen using forward pass output as input
	//
	Profiler::start("post_processing");
	postProcessingPipeline.render(viewMatrix, projectionMatrix, viewProjectionMatrix, targetProfile, FORWARD_PASS_OUTPUT, PRE_PASS_DEPTH_OUTPUT, VELOCITY_BUFFER_OUTPUT);
	Profiler::stop("post_processing");

	Profiler::stop("render");
}

unsigned int SceneViewPipeline::getOutput()
{
	return postProcessingPipeline.getOutput();
}

const Viewport& SceneViewPipeline::getViewport()
{
	return viewport;
}

void SceneViewPipeline::resizeViewport(float width, float height)
{
	// Set new viewport size
	viewport.width = width;
	viewport.height = height;

	// Recreate all passes to match new viewport
	destroyPasses();
	createPasses();

	Log::printProcessDone("Scene View", "Resize operation performed, various viewport dependant passes recreated");
}

void SceneViewPipeline::updateMsaaSamples(unsigned int _msaaSamples)
{
	// Set new msaa samples and recreate scene view forward pass
	msaaSamples = _msaaSamples;
	sceneViewForwardPass.destroy();
	sceneViewForwardPass.create(msaaSamples);
}

std::pair<TransformComponent, CameraComponent>& SceneViewPipeline::getFlyCamera()
{
	return flyCamera;
}

void SceneViewPipeline::createPasses()
{
	prePass.create();
	sceneViewForwardPass.create(msaaSamples);
	sceneViewForwardPass.setQuickGizmo(&imGizmo);
	ssaoPass.create();
	velocityBuffer.create();
	postProcessingPipeline.create();
}

void SceneViewPipeline::destroyPasses()
{
	prePass.destroy();
	sceneViewForwardPass.destroy();
	ssaoPass.destroy();
	velocityBuffer.destroy();
	postProcessingPipeline.destroy();
}