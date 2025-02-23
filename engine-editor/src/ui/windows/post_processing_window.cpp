#include <ui/windows/post_processing_window.h>

PostProcessingWindow::PostProcessingWindow(PostProcessing::Profile& targetProfile) : targetProfile(targetProfile)
{
}

void PostProcessingWindow::render()
{
	std::string title = UIUtils::windowTitle("Post Processing");
	ImGui::Begin(title.c_str(), nullptr, EditorFlag::standard);
	{
		IMComponents::headline("Post Processing", ICON_FA_SPARKLES);

		static bool post_processing_window_tmp = false;

		if (IMComponents::extendableSettings("Color Grading", post_processing_window_tmp, ICON_FA_PROJECTOR))
		{

			IMComponents::label("Contrast, Exposure & Gamma", EditorUI::getFonts().p_bold);
			ImGui::Dummy(ImVec2(0.0f, 5.0f));

			IMComponents::input("Contrast", targetProfile.color.contrast, 0.0001f);
			IMComponents::input("Exposure", targetProfile.color.exposure, 0.001f);
			IMComponents::input("Gamma", targetProfile.color.gamma, 0.001f);

			ImGui::Dummy(ImVec2(0.0f, 10.0f));
		}
		ImGui::Dummy(ImVec2(0.0f, 2.0f));

		if (IMComponents::extendableSettings("Motion Blur", targetProfile.motionBlur.enabled, ICON_FA_PROJECTOR))
		{
			IMComponents::label("Camera Motion Blur Settings", EditorUI::getFonts().p_bold);
			ImGui::Dummy(ImVec2(0.0f, 5.0f));

			IMComponents::input("Camera Blur", targetProfile.motionBlur.cameraEnabled);
			IMComponents::input("Intensity", targetProfile.motionBlur.cameraIntensity, 0.001f);
			IMComponents::input("Samples", targetProfile.motionBlur.cameraSamples, 0.1f);

			ImGui::Dummy(ImVec2(0.0f, 5.0f));
			IMComponents::label("Object Motion Blur Settings", EditorUI::getFonts().p_bold);
			ImGui::Dummy(ImVec2(0.0f, 5.0f));

			IMComponents::input("Object Blur", targetProfile.motionBlur.objectEnabled);
			IMComponents::input("Samples", targetProfile.motionBlur.objectSamples, 0.1f);

			ImGui::Dummy(ImVec2(0.0f, 10.0f));
		}
		ImGui::Dummy(ImVec2(0.0f, 2.0f));

		if (IMComponents::extendableSettings("Bloom", targetProfile.bloom.enabled, ICON_FA_PROJECTOR))
		{

			IMComponents::label("Bloom Settings", EditorUI::getFonts().p_bold);
			ImGui::Dummy(ImVec2(0.0f, 5.0f));

			IMComponents::input("Intensity", targetProfile.bloom.intensity, 0.001f);
			IMComponents::input("Threshold", targetProfile.bloom.threshold, 0.001f);
			IMComponents::input("Soft Threshold", targetProfile.bloom.softThreshold, 0.001f);
			IMComponents::input("Filter Radius", targetProfile.bloom.filterRadius, 0.001f);
			IMComponents::colorPicker("Bloom Color", targetProfile.bloom.color);

			ImGui::Dummy(ImVec2(0.0f, 5.0f));
			IMComponents::label("Lens Dirt Settings", EditorUI::getFonts().p_bold);
			ImGui::Dummy(ImVec2(0.0f, 5.0f));

			IMComponents::input("Enable Lens Dirt", targetProfile.bloom.lensDirtEnabled);
			IMComponents::input("Intensity", targetProfile.bloom.lensDirtIntensity, 0.01f);

			ImGui::Dummy(ImVec2(0.0f, 10.0f));
		}
		ImGui::Dummy(ImVec2(0.0f, 2.0f));

		if (IMComponents::extendableSettings("Vignette", targetProfile.vignette.enabled, ICON_FA_BAG_SHOPPING))
		{

			IMComponents::label("Vignette Settings", EditorUI::getFonts().p_bold);
			ImGui::Dummy(ImVec2(0.0f, 5.0f));

			IMComponents::input("Intensity", targetProfile.vignette.intensity, 0.01f);
			IMComponents::input("Radius", targetProfile.vignette.radius, 0.01f);
			IMComponents::input("Softness", targetProfile.vignette.softness, 0.01f);
			IMComponents::input("Roundness", targetProfile.vignette.roundness, 0.01f);
			IMComponents::colorPicker("Vignette Color", targetProfile.vignette.color);

			ImGui::Dummy(ImVec2(0.0f, 10.0f));
		}
		ImGui::Dummy(ImVec2(0.0f, 2.0f));

		if (IMComponents::extendableSettings("Chromatic Aberration", targetProfile.chromaticAberration.enabled, ICON_FA_BAG_SHOPPING))
		{

			IMComponents::label("Chromatic Aberration Settings", EditorUI::getFonts().p_bold);
			ImGui::Dummy(ImVec2(0.0f, 5.0f));
			
			IMComponents::input("Intensity", targetProfile.chromaticAberration.intensity, 0.01f);
			IMComponents::input("Iterations", targetProfile.chromaticAberration.iterations, 0.1f);

			ImGui::Dummy(ImVec2(0.0f, 10.0f));
		}
		ImGui::Dummy(ImVec2(0.0f, 2.0f));

		if (IMComponents::extendableSettings("Ambient Occlusion", targetProfile.ambientOcclusion.enabled, ICON_FA_PROJECTOR))
		{
			IMComponents::label("SSAO Settings", EditorUI::getFonts().p_bold);
			ImGui::Dummy(ImVec2(0.0f, 5.0f));

			IMComponents::input("Radius", targetProfile.ambientOcclusion.radius, 0.001f);
			IMComponents::input("Samples", targetProfile.ambientOcclusion.samples, 0.1f);
			IMComponents::input("Power", targetProfile.ambientOcclusion.power, 0.001f);
			IMComponents::input("Bias", targetProfile.ambientOcclusion.bias, 0.0001f);

			ImGui::Dummy(ImVec2(0.0f, 10.0f));
		}
		ImGui::Dummy(ImVec2(0.0f, 2.0f));
	}
	ImGui::End();
}

PostProcessing::Profile& PostProcessingWindow::getTargetProfile() {
	return targetProfile;
}

void PostProcessingWindow::updateTargetProfile(PostProcessing::Profile& profile) {
	targetProfile = profile;
}
