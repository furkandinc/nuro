#pragma once

#include <cstdint>
#include <glm.hpp>

#include "../core/viewport/viewport.h"
#include "../core/rendering/postprocessing/post_processing.h"

class Shader;

class MotionBlurPass
{
public:
	explicit MotionBlurPass(const Viewport& viewport);

	void create();
	void destroy();

	uint32_t render(const glm::mat4& view, const glm::mat4& projection, const glm::mat4& viewProjection, const PostProcessing::Profile& profile, const uint32_t hdrInput, const uint32_t depthInput, const uint32_t velocityBufferInput);

private:
	enum TextureUnits
	{
		HDR_UNIT,
		DEPTH_UNIT,
		VELOCITY_UNIT
	};

	const Viewport& viewport;

	uint32_t fbo;
	uint32_t output;

	Shader* shader;

	glm::mat4 previousViewProjectionMatrix;
};