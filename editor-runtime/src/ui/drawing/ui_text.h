#pragma once

#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui.h>
#include <cstdint>
#include <string>

#include "draw_collection.h"

struct UIText
{
	UIText(ImFont* font);

	//
	// PROPERTIES
	//

	// Pointer to font text is using
	ImFont* font;

	// Text value of element
	std::string text = "";

	// Position of text (may be controlled by parent)
	ImVec2 position = ImVec2(0.0f, 0.0f);

	// Color of text
	uint32_t color = IM_COL32(0, 0, 0, 255);

	// Horizontal and vertical padding of text
	ImVec2 padding = ImVec2(0.0f, 0.0f);

	// Text alignment within parent component if it has any
	DrawAlignment alignment = ALIGN_LEFT;

	//
	// VIRTUAL CONTENT ELEMENT FUNCTIONS
	//

	// Draws the ui text
	void draw(ImDrawList* drawList);

	// Returns the absolute size of the text in px
	ImVec2 getSize() const;
};
