#include "engine_dialog.h"

std::deque<int> EngineDialog::fpsCache = std::deque<int>(100);
float EngineDialog::fpsUpdateTimer = 0.0f;

void sparkline(const char* id, const float* values, int count, float min_v, float max_v, int offset, const ImVec4& col, const ImVec2& size) {
    ImPlot::PushStyleVar(ImPlotStyleVar_PlotPadding, ImVec2(0, 0));
    if (ImPlot::BeginPlot(id, size, ImPlotFlags_CanvasOnly)) {
        ImPlot::SetupAxes(nullptr, nullptr, ImPlotAxisFlags_NoDecorations, ImPlotAxisFlags_NoDecorations);
        ImPlot::SetupAxesLimits(0, count - 1, min_v, max_v, ImGuiCond_Always);
        ImPlot::SetNextLineStyle(col);
        ImPlot::SetNextFillStyle(col, 0.25);
        ImPlot::PlotLine(id, values, count, 1, 0, ImPlotLineFlags_Shaded, offset);
        ImPlot::EndPlot();
    }
    ImPlot::PopStyleVar();
}

ImVec4 lerpColors(const ImVec4& a, const ImVec4& b, float t) {
    return ImVec4(
        a.x + t * (b.x - a.x),
        a.y + t * (b.y - a.y),
        a.z + t * (b.z - a.z),
        a.w + t * (b.w - a.w)
    );
}

float remap(float old_min, float old_max, float new_min, float new_max, float value) {
    value = glm::clamp(value, old_min, old_max);
    return new_min + (value - old_min) * (new_max - new_min) / (old_max - old_min);
}

void EngineDialog::vec3_dialog(std::string name, glm::vec3& value, float min, float max) {
    ImGui::Begin(std::string("Vector3: " + name).c_str(), nullptr, EngineUI::windowFlags.fixed);

    ImGui::SliderFloat("X", &value.x, min, max);
    ImGui::SliderFloat("Y", &value.y, min, max);
    ImGui::SliderFloat("Z", &value.z, min, max);

    ImGui::End();
}

void EngineDialog::float_dialog(std::string name, float& value, float min, float max) {
    ImGui::Begin(std::string("Float: " + name).c_str(), nullptr, EngineUI::windowFlags.fixed);

    ImGui::SliderFloat(name.c_str(), &value, min, max);

    ImGui::End();
}

void EngineDialog::bool_dialog(std::string name, bool& value) {
    ImGui::Begin(std::string("Bool: " + name).c_str(), nullptr, EngineUI::windowFlags.fixed);

    ImGui::Checkbox(name.c_str(), &value);

    ImGui::End();
}

void EngineDialog::color_dialog(std::string name, glm::vec4& value) {
    ImGui::Begin(std::string("Color: " + name).c_str(), nullptr, EngineUI::windowFlags.fixed);

    ImGui::ColorPicker4(name.c_str(), (float*)&value);

    ImGui::End();
}

void EngineDialog::input_dialog(std::string name, std::vector<InputPair> inputs)
{
    ImGui::Begin(std::string("Float: " + name).c_str(), nullptr, EngineUI::windowFlags.fixed);

    for (int i = 0; i < inputs.size(); i++) {
        InputPair pair = inputs[i];
        ImGui::SliderFloat(pair.name.c_str(), &pair.floatValue, pair.sliderMin, pair.sliderMax);
    }

    ImGui::End();
}

void EngineDialog::plot_demo() {
    ImPlot::ShowDemoWindow();
}

void EngineDialog::show_diagnostics(float deltaTime, int fps, float averageFps) {
    ImGui::Begin("Diagnostics", nullptr, EngineUI::windowFlags.fixed);

    ImGui::Text("Average FPS:");
    ImGui::SameLine();
    ImGui::PushFont(EngineUI::fonts.uiBold);
    ImGui::Text("%.0f", averageFps);
    ImGui::PopFont();

    const int values = 100;
    const float updateRate = 0.025f;
    int maxValue = 1;
    if (fpsCache.size() > 0) maxValue = *std::max_element(fpsCache.begin(), fpsCache.end());

    fpsUpdateTimer += deltaTime;
    if (fpsUpdateTimer >= updateRate) {
        fpsCache.push_back(fps);

        if (fpsCache.size() > values) {
            fpsCache.pop_front();
        }

        fpsUpdateTimer = 0.0f;
    }

    float* data = new float[values];
    for (size_t i = 0; i < fpsCache.size(); ++i) {
        data[i] = fpsCache[i];
    }

    ImVec4 low = ImVec4(1.0f, 0.0f, 0.5f, 1.0f);
    ImVec4 high = ImVec4(0.0f, 1.0f, 0.5f, 1.0f);
    ImVec4 color = lerpColors(low, high, remap(0, maxValue, 0, 1, fps));
    sparkline("##spark", data, values, 0, maxValue, 0, color, ImVec2(120, 40));

    delete[] data;

    ImGui::End();

    if (ImGui::Begin("Tab Example"))  // Create a new window called "Tab Example"
    {
        // Create a tab bar
        if (ImGui::BeginTabBar("Tabs"))
        {
            // Tab 1
            if (ImGui::BeginTabItem("Tab 1"))
            {
                ImGui::Text("This is the content of Tab 1");
                ImGui::EndTabItem();  // Close the first tab item
            }

            // Tab 2
            if (ImGui::BeginTabItem("Tab 2"))
            {
                ImGui::Text("This is the content of Tab 2");
                ImGui::EndTabItem();  // Close the second tab item
            }

            // Tab 3
            if (ImGui::BeginTabItem("Tab 3"))
            {
                ImGui::Text("This is the content of Tab 3");
                ImGui::EndTabItem();  // Close the third tab item
            }

            ImGui::EndTabBar();  // Close the tab bar
        }
    }
    ImGui::End();
}