#include "bloom_pass.h"

#include <glad/glad.h>

#include <rendering/shader/shader_pool.h>
#include <rendering/shader/shader.h>
#include <rendering/primitives/global_quad.h>
#include <utils/console.h>

BloomPass::BloomPass(const Viewport& viewport) : viewport(viewport),
threshold(0.0f),
softThreshold(0.0f),
filterRadius(0.0f),
mipChain(),
iViewportSize(0, 0),
fViewportSize(0.0f, 0.0f),
inversedViewportSize(0, 0),
framebuffer(0),
prefilterOutput(0),
prefilterShader(ShaderPool::empty()),
downsamplingShader(ShaderPool::empty()),
upsamplingShader(ShaderPool::empty())
{
}

void BloomPass::create(const uint32_t mipDepth)
{
	// Load shaders
	prefilterShader = ShaderPool::get("bloom_prefilter");
	downsamplingShader = ShaderPool::get("bloom_downsampling");
	upsamplingShader = ShaderPool::get("bloom_upsampling");

	// Set static prefilter uniforms
	prefilterShader->bind();
	prefilterShader->setInt("inputTexture", 0);

	// Set static downsampling uniforms
	downsamplingShader->bind();
	downsamplingShader->setInt("inputTexture", 0);

	// Set static upsampling uniforms
	upsamplingShader->bind();
	upsamplingShader->setInt("inputTexture", 0);

	// Get initial viewport size
	iViewportSize = viewport.getResolution_i();
	fViewportSize = viewport.getResolution();
	inversedViewportSize = 1.0f / fViewportSize;

	// Get initial viewport size
	glm::ivec2 iMipSize = viewport.getResolution_i();
	glm::vec2 fMipSize = viewport.getResolution();

	// Generate framebuffer
	glGenFramebuffers(1, &framebuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

	// Generate prefilter texture
	glGenTextures(1, &prefilterOutput);
	glBindTexture(GL_TEXTURE_2D, prefilterOutput);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, viewport.getWidth_gl(), viewport.getHeight_gl(), 0, GL_RGBA, GL_FLOAT, nullptr);

	// Set prefilter texture parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	// Generate all bloom mips
	for (uint32_t i = 0; i < mipDepth; i++)
	{
		BloomPass::Mip mip;

		// Halve mips size
		iMipSize /= 2;
		fMipSize *= 0.5f;

		mip.iSize = iMipSize;
		mip.fSize = fMipSize;
		mip.inversedSize = 1.0f / fMipSize;

		// Generate mips texture
		glGenTextures(1, &mip.texture);
		glBindTexture(GL_TEXTURE_2D, mip.texture);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, iMipSize.x, iMipSize.y, 0, GL_RGBA, GL_FLOAT, nullptr);

		// Set mip textures parameters
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		// Add generated mip to bloom mip chain
		mipChain.emplace_back(mip);
	}

	// Set framebuffer attachments, set first mip to be the color attachment
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, mipChain[0].texture, 0);

	uint32_t fboAttachments[1] = {
		GL_COLOR_ATTACHMENT0 };
	glDrawBuffers(1, fboAttachments);

	// Check for framebuffer errors
	GLenum fboStatus = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if (fboStatus != GL_FRAMEBUFFER_COMPLETE)
	{
		Console::out::warning("Bloom Pass", "Issue while generating framebuffer: " + std::to_string(fboStatus));
	}

	// Unbind framebuffer
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void BloomPass::destroy()
{
	// Delete prefilter texture
	glDeleteTextures(1, &prefilterOutput);
	prefilterOutput = 0;

	// Delete all mipmap texture
	for (auto& mip : mipChain)
	{
		glDeleteTextures(1, &mip.texture);
	}

	// Clear mipchain
	mipChain.clear();

	// Delete framebuffer
	glDeleteFramebuffers(1, &framebuffer);
	framebuffer = 0;

	// Remove shaders
	prefilterShader = nullptr;
	downsamplingShader = nullptr;
	upsamplingShader = nullptr;
}

uint32_t BloomPass::render(const uint32_t hdrInput)
{
	// Bind bloom framebuffer
	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

	// Perform prefiltering pass
	uint32_t PREFILTERING_PASS_OUTPUT = prefilteringPass(hdrInput);

	// Perform downsampling pass
	downsamplingPass(PREFILTERING_PASS_OUTPUT);

	// Perform upsampling pass
	upsamplingPass();

	// Unbind framebuffer to restore original viewport
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0, 0, iViewportSize.x, iViewportSize.y);

	// Return texture of first bloom mip (the texture being rendered to)
	return mipChain[0].texture;
}

uint32_t BloomPass::prefilteringPass(const uint32_t hdrInput)
{
	// Set prefilter uniforms
	prefilterShader->bind();
	prefilterShader->setFloat("threshold", threshold);
	prefilterShader->setFloat("softThreshold", softThreshold);

	// Bind input texture for prefilter pass
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, hdrInput);

	// Set prefilter target texture as framebuffer render target
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, prefilterOutput, 0);

	// Bind and render to quad
	GlobalQuad::bind();
	GlobalQuad::render();

	// Return prefiltering pass target (now the output after rendering)
	return prefilterOutput;
}

void BloomPass::downsamplingPass(const uint32_t hdrInput)
{
	// Set downsampling uniforms
	downsamplingShader->bind();
	downsamplingShader->setVec2("inversedResolution", inversedViewportSize);

	// Bind input as initial texture input
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, hdrInput);

	// Downsample through mip chain
	for (int32_t i = 0; i < mipChain.size(); i++)
	{
		// Get current mip
		const BloomPass::Mip& mip = mipChain[i];

		// Set viewport and framebuffer rendering target according to current mip
		glViewport(0, 0, static_cast<GLsizei>(mip.fSize.x), static_cast<GLsizei>(mip.fSize.y));
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, mip.texture, 0);

		// Bind and render to quad
		GlobalQuad::bind();
		GlobalQuad::render();

		// Set inversed resolution for next downsample iteration
		downsamplingShader->setVec2("inversedResolution", mip.inversedSize);

		// Bind mip texture for next downsample iteration
		glBindTexture(GL_TEXTURE_2D, mip.texture);
	}
}

void BloomPass::upsamplingPass()
{
	// Get viewport aspect ratio
	const float aspectRatio = fViewportSize.x / fViewportSize.y;

	// Set downsampling uniforms
	upsamplingShader->bind();
	upsamplingShader->setFloat("filterRadius", filterRadius);
	upsamplingShader->setFloat("aspectRatio", aspectRatio);

	// Enable additive blending
	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE, GL_ONE);
	glBlendEquation(GL_FUNC_ADD);

	// Upsample through mip chain
	for (size_t i = mipChain.size() - 1; i > 0; i--)
	{
		// Get current mip and target mip for current downsampling iteration
		const BloomPass::Mip& mip = mipChain[i];
		const BloomPass::Mip& targetMip = mipChain[i - 1];

		// Set input texture for next render to be current mips texture
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, mip.texture);

		// Set viewport and set render target
		glViewport(0, 0, static_cast<GLsizei>(targetMip.fSize.x), static_cast<GLsizei>(targetMip.fSize.y));
		glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, targetMip.texture, 0);

		// Bind and render to quad
		GlobalQuad::bind();
		GlobalQuad::render();
	}

	// Disable additive blending
	glDisable(GL_BLEND);
}