#pragma once

#include <deque>

#include "../engine/engineui/engine_ui.h"

class DiagnosticsWindow : public EngineWindow
{
public:
	void prepare();
private:
	static std::deque<int> fpsCache;
	static float fpsUpdateTimer;
};

