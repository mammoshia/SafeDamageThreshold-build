#include "../src/DamageMath.h"

#include <cassert>
#include <cmath>

int main()
{
	using SafeDT::ApplyThreshold;

	const auto stopped = ApplyThreshold(15.0F, 1000.0F, 0.02F, 100.0F);
	assert(stopped.threshold == 20.0F);
	assert(stopped.after == 0.0F);
	assert(stopped.ratio == 0.0F);

	const auto passed = ApplyThreshold(50.0F, 1000.0F, 0.02F, 100.0F);
	assert(passed.threshold == 20.0F);
	assert(passed.after == 30.0F);
	assert(std::abs(passed.ratio - 0.6F) < 0.0001F);

	const auto negative = ApplyThreshold(-25.0F, 1000.0F, 0.02F, 100.0F);
	assert(negative.before == 0.0F);
	assert(negative.after == 0.0F);

	const auto capped = ApplyThreshold(1000.0F, 10000.0F, 0.02F, 100.0F);
	assert(capped.threshold == 100.0F);
	assert(capped.after == 900.0F);
}

