#pragma once

#include <ui/windows/editor_window.h>

#include <rendering/postprocessing/post_processing.h>

class PostProcessingWindow : public EditorWindow
{
public:
	PostProcessingWindow(PostProcessing::Profile& targetProfile);

	void render() override;

	PostProcessing::Profile& getTargetProfile();
	void updateTargetProfile(PostProcessing::Profile& profile);

private:
	PostProcessing::Profile& targetProfile;
};
