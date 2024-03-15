#include "debug.h"

namespace Cme
{
	Cme::DebugGroup::DebugGroup(const char* name)
	{
		glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, -1, name);
	}

	DebugGroup::~DebugGroup() { glPopDebugGroup(); }

}  // namespace Cme