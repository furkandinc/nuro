#include "engine_ui.h"

std::vector<EngineWindow*> EngineUI::windows = std::vector<EngineWindow*>();

Sizing EngineUI::sizing;
Colors EngineUI::colors;
WindowFlags EngineUI::windowFlags;
Fonts EngineUI::fonts;

void EngineUI::setup() {
	IMGUI_CHECKVERSION();

	ImGui::CreateContext();
	ImPlot::CreateContext();

	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

	// Load default font
	fonts.uiRegular = io.Fonts->AddFontFromFileTTF("./resources/fonts/Inter_18pt-Light.ttf", sizing.regularFontSize);

	// Merge icons into regularFontSize font
	float iconFontSize = sizing.iconFontSize * 2.0f / 3.0f;
	static const ImWchar icons_ranges[] = { ICON_MIN_FA, ICON_MAX_FA, 0 };
	ImFontConfig icons_config;
	icons_config.MergeMode = true;
	icons_config.PixelSnapH = true;
	icons_config.GlyphMinAdvanceX = iconFontSize;
	fonts.uiIcons = io.Fonts->AddFontFromFileTTF("./resources/fonts/fa-solid-900.ttf", iconFontSize, &icons_config, icons_ranges);

	// Load other fonts
	fonts.uiBold = io.Fonts->AddFontFromFileTTF("./resources/fonts/Inter_18pt-SemiBold.ttf", sizing.regularFontSize);
	fonts.uiHeadline = io.Fonts->AddFontFromFileTTF("./resources/fonts/Inter_18pt-SemiBold.ttf", sizing.headlineFontSize);

	ImGui::StyleColorsDark();

	ImGuiStyle& style = ImGui::GetStyle();

	style.ButtonTextAlign = ImVec2(0.0f, 0.0f);
	style.SelectableTextAlign = ImVec2(0.0f, 0.0f);
	style.SeparatorTextAlign = ImVec2(0.0f, 0.0f);
	style.TableAngledHeadersTextAlign = ImVec2(0.0f, 0.0f);
	style.WindowTitleAlign = ImVec2(0.0f, 0.0f);

	style.WindowPadding = ImVec2(20.0f, 30.0f);
	style.WindowRounding = 4.0f;
	style.WindowMenuButtonPosition = ImGuiDir_None;
	style.WindowBorderSize = 0.5f;

	style.FramePadding = ImVec2(3.0f, 3.0f);
	style.FrameRounding = 2.0f;

	style.GrabMinSize = 5.0f;
	style.GrabRounding = 0.0f;

	style.TabRounding = 1.0f;

	style.ItemSpacing = ImVec2(4.0f, 8.0f);

	ImVec4* imguiColors = style.Colors;

	imguiColors[ImGuiCol_WindowBg] = colors.background;

	imguiColors[ImGuiCol_TitleBg] = colors.background;
	imguiColors[ImGuiCol_TitleBgActive] = colors.background;
	imguiColors[ImGuiCol_TitleBgCollapsed] = colors.background;

	imguiColors[ImGuiCol_Text] = colors.text;
	imguiColors[ImGuiCol_Button] = colors.element;
	imguiColors[ImGuiCol_ButtonHovered] = colors.elementHovered;
	imguiColors[ImGuiCol_ButtonActive] = colors.elementActive;

	imguiColors[ImGuiCol_FrameBg] = colors.element;
	imguiColors[ImGuiCol_FrameBgHovered] = colors.elementHovered;
	imguiColors[ImGuiCol_FrameBgActive] = colors.elementActive;

	imguiColors[ImGuiCol_SliderGrab] = colors.elementComponent;
	imguiColors[ImGuiCol_SliderGrabActive] = colors.elementComponent;
	imguiColors[ImGuiCol_CheckMark] = colors.elementComponent;

	imguiColors[ImGuiCol_ResizeGrip] = colors.element;
	imguiColors[ImGuiCol_ResizeGripActive] = colors.elementActive;
	imguiColors[ImGuiCol_ResizeGripHovered] = colors.elementHovered;

	imguiColors[ImGuiCol_SeparatorHovered] = colors.element;
	imguiColors[ImGuiCol_SeparatorActive] = colors.elementActive;

	imguiColors[ImGuiCol_Border] = colors.borderColor;

	imguiColors[ImGuiCol_Tab] = colors.elementActive;
	imguiColors[ImGuiCol_TabHovered] = colors.elementActive;
	imguiColors[ImGuiCol_TabActive] = colors.elementActive;

	imguiColors[ImGuiCol_TabDimmed] = colors.elementActive;
	imguiColors[ImGuiCol_TabDimmedSelected] = colors.elementActive;
	imguiColors[ImGuiCol_TabDimmedSelectedOverline] = colors.elementActive;

	imguiColors[ImGuiCol_DockingPreview] = colors.elementActive;
	imguiColors[ImGuiCol_DockingEmptyBg] = colors.elementActive;

	ImGui_ImplGlfw_InitForOpenGL(Window::glfw, true);
	ImGui_ImplOpenGL3_Init("#version 460");

	SceneView* sceneView = new SceneView();
	windows.push_back(sceneView);
}

void EngineUI::newFrame() {
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();
}

void EngineUI::render() {

	/* CREATE MAIN VIEWPORT DOCKSPACE */
	ImGuiViewport* viewport = ImGui::GetMainViewport();

	ImGui::SetNextWindowPos(viewport->Pos);
	ImGui::SetNextWindowSize(viewport->Size);
	ImGui::SetNextWindowViewport(viewport->ID);

	ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoTitleBar |
		ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize |
		ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBringToFrontOnFocus |
		ImGuiWindowFlags_NoNavFocus | ImGuiWindowFlags_NoBackground;

	ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
	ImGui::Begin("Fullscreen Dockspace", nullptr, window_flags);

	ImGuiID dockspace_id = ImGui::GetID("MyFullscreenDockspace");
	ImGui::DockSpace(dockspace_id, ImVec2(0, 0), ImGuiDockNodeFlags_PassthruCentralNode);

	ImGui::End();
	ImGui::PopStyleVar(3);

	/* PREPARE ALL WINDOWS */
	for (int i = 0; i < windows.size(); i++) {
		// windows[i]->prepare();
	}

	/* RENDERING AND DRAW CALLS */
	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

ImVec4 EngineUI::lighten(ImVec4 color, float amount) {
	float factor = 1.0f + amount;
	return ImVec4(color.x * factor, color.y * factor, color.z * factor, color.w);
}

ImVec4 EngineUI::darken(ImVec4 color, float amount) {
	float factor = 1.0f - amount;
	return ImVec4(color.x * factor, color.y * factor, color.z * factor, color.w);
}