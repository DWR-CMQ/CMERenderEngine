#include "random.h"

namespace Cme
{
	unsigned int SeedGenerator::generateTrueRandomSeed()
	{
		std::random_device rd_;
		return rd_();
	}

}  // namespace Cme
