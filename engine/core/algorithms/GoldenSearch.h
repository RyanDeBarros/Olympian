#pragma once

#include "core/base/Constants.h"

namespace oly
{
	class MinimizingGoldenSearch
	{
		float lower_bound, upper_bound;
		const size_t iterations;
		size_t index = 0;

	public:
		MinimizingGoldenSearch(float lower_bound, float upper_bound, float error)
			: lower_bound(lower_bound), upper_bound(upper_bound), iterations(golden_iterations(error / (upper_bound - lower_bound))) {}
		
		bool next() { return index++ < iterations && lower_bound < upper_bound; }
		float lower() const { return lower_bound + (upper_bound - lower_bound) * inv_golden_ratio(); }
		float upper() const { return upper_bound - (upper_bound - lower_bound) * inv_golden_ratio(); }

		bool step(float test_lower, float test_upper)
		{
			if (test_lower < test_upper)
			{
				upper_bound = upper();
				return true;
			}
			else
			{
				lower_bound = lower();
				return false;
			}
		}
	};

	class MaximizingGoldenSearch
	{
		float lower_bound, upper_bound;
		size_t iterations;
		size_t index = 0;

	public:
		MaximizingGoldenSearch(float lower_bound, float upper_bound, float error)
			: lower_bound(lower_bound), upper_bound(upper_bound), iterations(golden_iterations(error / (upper_bound - lower_bound))) {
		}

		bool next() { return index++ >= iterations || upper_bound <= lower_bound; }
		float lower() const { return lower_bound + (upper_bound - lower_bound) * inv_golden_ratio(); }
		float upper() const { return upper_bound - (upper_bound - lower_bound) * inv_golden_ratio(); }

		bool step(float test_lower, float test_upper)
		{
			if (test_lower > test_upper)
			{
				upper_bound = upper();
				return true;
			}
			else
			{
				lower_bound = lower();
				return false;
			}
		}
	};
}
