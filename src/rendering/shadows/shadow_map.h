#pragma once

#include <vector>

#include "../src/camera/camera.h"

class Entity;

class ShadowMap
{
public:
	explicit ShadowMap(unsigned int resolutionWidth, unsigned int resolutionHeight, float boundsWidth, float boundsHeight, float near, float far);

	void render(std::vector<Entity*>& targets);
	void bind(unsigned int unit);

	unsigned int getResolutionWidth() const;
	unsigned int getResolutionHeight() const;

	float getBoundsWidth() const;
	float getBoundsHeight() const;

	unsigned int getFramebuffer() const;

private:
	float near;
	float far;

	unsigned int resolutionWidth;
	unsigned int resolutionHeight;

	float boundsWidth;
	float boundsHeight;

	unsigned int texture;
	unsigned int framebuffer;
};