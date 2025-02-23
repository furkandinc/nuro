#include <ui/misc/ui_flex.h>

#include <imgui.h>
#include <cmath>

//
// DEPRECATED, BAD CODE!
//

bool UIFlex::debugMode = false;
FlexBuffer UIFlex::lastFlex;

float UIFlex::mapAlignment(Alignment alignment)
{
	switch (alignment) {
	case Alignment::START:
		return 0.0f;
	case Alignment::CENTER:
		return 0.5f;
	case Alignment::END:
		return 1.0f;
	}
}

ImVec2 UIFlex::getFlexRowSize(float width, float height)
{
	float windowWidth = ImGui::GetWindowWidth();
	float contentWidth = ImGui::GetContentRegionAvail().x;
	float rightPadding = windowWidth - contentWidth;
	float fullWidth = windowWidth - rightPadding;
	ImVec2 layoutSize = ImVec2(width, height);
	return layoutSize;
}

void UIFlex::beginFlex(ImGuiID id, FlexType type, float width, float height, Justification justification, Alignment alignment, float spacing, Margin margin)
{
	if (type == FlexType::ROW)
	{
		static ImVec2 itemSpacing = ImVec2(spacing, 0.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(floorf(itemSpacing.x), floorf(itemSpacing.y)));

		ImGui::Dummy(ImVec2(0.0f, margin.top));

		ImVec2 rowSize = getFlexRowSize(width, height);
		//ImGui::BeginHorizontal(id, rowSize, mapAlignment(alignment)); // TODO incorrect ImGui version thru cmake

		if (margin.left != 0)
			ImGui::Dummy(ImVec2(margin.left, 0.0f));

		if (justification == Justification::CENTER || justification == Justification::END)
		{
			//ImGui::Spring(0.5f); // TODO incorrect ImGui version thru cmake
		}

		lastFlex.type = type;
		lastFlex.width = width;
		lastFlex.height = height;
		lastFlex.justification = justification;
		lastFlex.alignment = alignment;
		lastFlex.spacing = spacing;
		lastFlex.margin = margin;
	}
}

void UIFlex::endFlex()
{
	if (lastFlex.type == FlexType::ROW)
	{
		if (lastFlex.margin.right != 0)
			ImGui::Dummy(ImVec2(lastFlex.margin.right, 0.0f));

		if (lastFlex.justification == Justification::CENTER || lastFlex.justification == Justification::START)
		{
			//ImGui::Spring(0.5f); // TODO incorrect ImGui version thru cmake
		}

		//ImGui::EndHorizontal(); // TODO incorrect ImGui version thru cmake
		ImGui::PopStyleVar();

		ImGui::Dummy(ImVec2(0.0f, lastFlex.margin.bottom));

		if (debugMode)
		{
			ImDrawList* draw_list = ImGui::GetWindowDrawList();
			draw_list->AddRect(ImGui::GetItemRectMin(), ImGui::GetItemRectMax(), ImGui::GetColorU32(ImGuiCol_Border));
		}
	}
}
