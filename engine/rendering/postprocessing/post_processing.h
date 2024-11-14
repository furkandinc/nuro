#pragma once

#include <glad/glad.h>
#include <glm.hpp>

#include "../engine/window/window.h"
#include "../engine/utils/log.h"
#include "../engine/rendering/texture/texture.h"
#include "../engine/rendering/shader/shader.h"
#include "../engine/rendering/shader/shader_builder.h"
#include "../engine/rendering/primitives/quad.h"
#include "../engine/rendering/postprocessing/debug/debug_pass.h"
#include "../engine/rendering/postprocessing/bloom/bloom_pass.h"

enum FinalPassTextureSlots {
	HDR_BUFFER_UNIT,
	BLOOM_BUFFER_UNIT,
	LENS_DIRT_UNIT
};

struct PostProcessingConfiguration {
	bool colorGrading = false;

	float exposure = 1.0f;
	float contrast = 1.004f;
	float gamma = 2.2f;

	bool bloom = true;
	float bloomIntensity = 0.25f;
	float bloomColor[3] = { 1.0f, 1.0f, 1.0f };
	float bloomThreshold = 0.2f;
	float bloomSoftThreshold = 0.0f;
	float bloomFilterRadius = 0.0f;
	unsigned int bloomMipDepth = 16;
	bool lensDirt = false;
	Texture* lensDirtTexture = nullptr;
	float lensDirtIntensity = 0.0f;

	bool chromaticAberration = true;
	float chromaticAberrationIntensity = 0.5f;
	float chromaticAberrationRange = 0.2f;
	float chromaticAberrationRedOffset = 0.01f;
	float chromaticAberrationBlueOffset = 0.01f;

	bool vignette = true;
	float vignetteIntensity = 1.0f;
	float vignetteColor[3] = { 0.0f, 0.0f, 0.0f };
	float vignetteRadius = 0.68f;
	float vignetteSoftness = 0.35f;
	float vignetteRoundness = 1.8f;

	bool ambientOcclusion = false;
};

class PostProcessing
{
public:
	static void setup();
	static void render(unsigned int input);

	static PostProcessingConfiguration configuration;

	static unsigned int getOutput();
private:
	static void syncConfiguration();

	static Shader* finalPassShader;

	static PostProcessingConfiguration defaultConfiguration;

	static unsigned int fbo;
	static unsigned int output;
};