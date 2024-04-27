#pragma once

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <glad/glad.h>

namespace Cme 
{
	// RAII debugging group marker.
	class DebugGroup 
	{
	public:
		DebugGroup(const char* name);
		~DebugGroup();
	};
}  // namespace Cme

