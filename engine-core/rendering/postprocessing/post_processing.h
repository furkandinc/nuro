#pragma once

#include <cstdint>
#include <glm.hpp>

#include "../src/core/rendering/texture/texture.h"

namespace PostProcessing
{

	struct Color {

		float exposure = 1.0f;
		float contrast = 1.004f;
		float gamma = 2.2f;

	};

	struct MotionBlur {

		bool enabled = true;

		bool cameraEnabled = true;
		float cameraIntensity = 0.8f;
		int32_t cameraSamples = 32;

		bool objectEnabled = false;
		int32_t objectSamples = 16;

	};

	struct Bloom {

		bool enabled = true;
		float intensity = 0.075f;
		glm::vec3 color = glm::vec3(1.0f);
		float threshold = 0.465f;
		float softThreshold = 0.0f;
		float filterRadius = 0.0f;

		bool lensDirtEnabled = false;
		uint32_t lensDirtTexture = 0;
		float lensDirtIntensity = 0.0f;

	};

	struct ChromaticAberration {

		bool enabled = true;
		float intensity = -0.155;
		int32_t iterations = 12;

	};

	struct Vignette {

		bool enabled = true;
		float intensity = 1.0f;
		glm::vec3 color = glm::vec3(0.0f);
		float radius = 0.74f;
		float softness = 0.42f;
		float roundness = 1.8f;

	};

	struct AmbientOcclusion {

		bool enabled = false;
		float radius = 0.2f;
		int32_t samples = 64;
		float power = 20.0f;
		float bias = 0.03f;

	};

	struct Profile {

		Color color;
		MotionBlur motionBlur;
		Bloom bloom;
		ChromaticAberration chromaticAberration;
		Vignette vignette;
		AmbientOcclusion ambientOcclusion;

	};

}