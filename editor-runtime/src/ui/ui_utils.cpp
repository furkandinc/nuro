#include "ui_utils.h"

#include <math.h>

#include "../core/input/cursor.h"

namespace UIUtils {

	ImVec4 lighten(ImVec4 color, float amount)
	{
		float factor = 1.0f + amount;
		return ImVec4(color.x * factor, color.y * factor, color.z * factor, color.w);
	}

	ImU32 lighten(ImU32 color, float amount)
	{
		// Extract RGBA components
		float r = (float)((color >> 16) & 0xFF) / 255.0f;
		float g = (float)((color >> 8) & 0xFF) / 255.0f;
		float b = (float)(color & 0xFF) / 255.0f;

		// Apply lighten factor to RGB
		float factor = 1.0f + amount;
		r = fminf(r * factor, 1.0f); // Ensure we don't exceed 1.0
		g = fminf(g * factor, 1.0f);
		b = fminf(b * factor, 1.0f);

		// Convert back to ImU32 (with clamped RGB values)
		return ((int)(r * 255) << 16) | ((int)(g * 255) << 8) | ((int)(b * 255)) | (color & 0xFF000000);
	}

	ImU32 darken(ImU32 color, float amount)
	{
		// Extract RGBA components
		float r = (float)((color >> 16) & 0xFF) / 255.0f;
		float g = (float)((color >> 8) & 0xFF) / 255.0f;
		float b = (float)(color & 0xFF) / 255.0f;

		// Apply darken factor to RGB
		float factor = 1.0f - amount;
		r = fmaxf(r * factor, 0.0f); // Ensure we don't go below 0
		g = fmaxf(g * factor, 0.0f);
		b = fmaxf(b * factor, 0.0f);

		// Convert back to ImU32 (with clamped RGB values)
		return ((int)(r * 255) << 16) | ((int)(g * 255) << 8) | ((int)(b * 255)) | (color & 0xFF000000);
	}

	ImVec4 darken(ImVec4 color, float amount)
	{
		float factor = 1.0f - amount;
		return ImVec4(color.x * factor, color.y * factor, color.z * factor, color.w);
	}

	float getChildScrollValue()
	{
		// Get the current scroll position of the child window (if it's a child)
		float scrollY = ImGui::GetScrollY();

		// Get the maximum scroll position of the child window
		float maxScrollY = ImGui::GetScrollMaxY();

		// Avoid division by zero
		if (maxScrollY <= 0.0f) return 0.0f;

		// Return the relative scroll height as a value between 0.0 and 1.0
		return scrollY / maxScrollY;
	}

	glm::vec2 keepCursorInBounds(glm::vec4 bounds, bool& positionedCursor, float offset)
	{
		glm::vec2 currentPos = Cursor::getPosition();
		glm::vec2 updatedPos = currentPos;

		glm::vec2 min = glm::vec2(bounds.x, bounds.y);
		glm::vec2 max = glm::vec2(bounds.z, bounds.w);

		// Horizontal boundaries
		if (currentPos.x < min.x + offset) {
			updatedPos = glm::vec2(max.x - offset, currentPos.y);
			Cursor::setPosition(updatedPos);
			positionedCursor = true;
		}
		else if (currentPos.x > max.x - offset) {
			updatedPos = glm::vec2(min.x + offset, currentPos.y);
			Cursor::setPosition(updatedPos);
			positionedCursor = true;
		}

		// Vertical boundaries
		if (currentPos.y < min.y + offset) {
			updatedPos = glm::vec2(currentPos.x, max.y - offset);
			Cursor::setPosition(updatedPos);
			positionedCursor = true;
		}
		else if (currentPos.y > max.y - offset) {
			updatedPos = glm::vec2(currentPos.x, min.y + offset);
			Cursor::setPosition(updatedPos);
			positionedCursor = true;
		}

		return updatedPos;
	}

	void calculateAspectFitting(float aspectRatio, ImVec2& size, ImVec2& offset)
	{
		// Get content region avail for game view
		ImVec2 contentRegionAvail = ImGui::GetContentRegionAvail();

		// Determine the maximum size that maintains the 16:9 aspect ratio, scale by width
		size.x = contentRegionAvail.x;
		size.y = contentRegionAvail.x / aspectRatio;

		// Calculated height exceeds available height, scale by height instead
		if (size.y > contentRegionAvail.y) {
			size.y = contentRegionAvail.y;
			size.x = contentRegionAvail.y * aspectRatio;
		}

		// Set offset of game view render target to make sure its centered
		offset = ImVec2((contentRegionAvail.x - size.x) * 0.5f, (contentRegionAvail.y - size.y) * 0.5f);
	}

}