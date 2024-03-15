#ifndef QUARKGL_DEBUG_H_
#define QUARKGL_DEBUG_H_

// clang-format off
// Must precede glfw/glad, to include OpenGL functions.
#include "core.h"
// clang-format on

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

#endif
