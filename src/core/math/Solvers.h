#pragma once

#include <stdexcept>

namespace oly
{
	namespace math
	{
		namespace solver
		{
			// TODO use ErrorCode
			struct no_solution : public std::exception { no_solution() : std::exception("no solution exists") {} };
			struct infinite_solutions : public std::exception { infinite_solutions() : std::exception("infinitely many solutions exist") {} };

			struct LinearCosine
			{
				// Ax + cos(Bx + C) + D = 0
				float A, B, C, D;
				unsigned int _iterations = 3;

				float solve() const;
			};
		}
	}
}
