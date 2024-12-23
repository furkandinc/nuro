#include "pre_pass.h"

#include <glad/glad.h>

#include "../core/rendering/shader/shader_pool.h"
#include "../core/utils/log.h"
#include "../core/rendering/core/transformation.h"
#include "../core/rendering/model/mesh.h"
#include "../core/ecs/ecs_collection.h"

PrePass::PrePass(const Viewport& viewport) : viewport(viewport),
fbo(0),
depthOutput(0),
normalOutput(0),
prePassShader(nullptr)
{
}

void PrePass::create()
{
	// Get pre pass shader
	prePassShader = ShaderPool::get("pre_pass");

	// Generate framebuffer
	glGenFramebuffers(1, &fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);

	// Generate depth output
	glGenTextures(1, &depthOutput);
	glBindTexture(GL_TEXTURE_2D, depthOutput);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, viewport.width, viewport.height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);

	// Set depth output parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	// Set depth output as rendering target
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthOutput, 0);

	// Generate normal output
	glGenTextures(1, &normalOutput);
	glBindTexture(GL_TEXTURE_2D, normalOutput);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, viewport.width, viewport.height, 0, GL_RGB, GL_FLOAT, nullptr);

	// Set normal output parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	// Set normal output as rendering target
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, normalOutput, 0);

	// Check for framebuffer errors
	GLenum fboStatus = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if (fboStatus != GL_FRAMEBUFFER_COMPLETE)
	{
		Log::printError("Framebuffer", "Error generating bloom fbo: " + std::to_string(fboStatus));
	}

	// Unbind fbo
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void PrePass::destroy() {
	// Delete depth output texture
	glDeleteTextures(1, &depthOutput);
	depthOutput = 0;

	// Delete normal output texture
	glDeleteTextures(1, &normalOutput);
	normalOutput = 0;

	// Delete framebuffer
	glDeleteFramebuffers(1, &fbo);
	fbo = 0;

	// Remove shaders
	prePassShader = nullptr;
}

void PrePass::render(glm::mat4 viewProjection, glm::mat3 viewNormal)
{
	// Set viewport for upcoming pre pass
	glViewport(0, 0, viewport.width, viewport.height);

	// Bind pre pass framebuffer
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);

	// Clear color and depth buffer
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Enable depth testing
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);

	// Cull backfaces
	glCullFace(GL_BACK);

	// Bind pre pass shader
	prePassShader->bind();

	// Pre pass render each entity
	auto targets = ECS::gRegistry.view<TransformComponent, MeshRendererComponent>();
	for (auto [entity, transform, renderer] : targets.each()) {
		// Recalculate transforms matrix cache for all upcoming passes of current frame
		transform.model = Transformation::model(transform);
		transform.mvp = viewProjection * transform.model;

		// Bind mesh
		glBindVertexArray(renderer.mesh.getVAO());

		// Set depth pre pass shader uniforms
		prePassShader->setMatrix4("mvpMatrix", transform.mvp);
		prePassShader->setMatrix3("viewNormalMatrix", viewNormal);

		// Render mesh
		glDrawElements(GL_TRIANGLES, renderer.mesh.getIndiceCount(), GL_UNSIGNED_INT, 0);
	}
}

uint32_t PrePass::getDepthOutput()
{
	// Return pre pass depth output
	return depthOutput;
}

uint32_t PrePass::getNormalOutput()
{
	// Return pre pass normal output
	return normalOutput;
}