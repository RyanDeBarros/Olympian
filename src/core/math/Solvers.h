#pragma once

#include <stdexcept>

namespace oly::math::solver
{
	struct LinearCosine
	{
		// Ax + cos(Bx + C) + D = 0
		float A, B, C, D;
		unsigned int _iterations = 3;

		float solve() const;
	};
}
