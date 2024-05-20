#pragma once

#include "../common_helper.h"

namespace Cme
{
	class UI
	{
	public:
		static void SetupUI(GLFWwindow* window);
		static void RenderUI(ModelRenderOptions& opts, Cme::Camera camera);

	};
}


