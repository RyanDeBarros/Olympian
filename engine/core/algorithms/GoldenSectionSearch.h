#pragma once

#include "core/base/Constants.h"

#include <functional>

namespace oly
{
	struct GoldenSearchResult
	{
		float input = 0.0f, output = 0.0f;
	};

	struct EarlyExitGoldenSearchResult
	{
		bool early_exited = true;
		float input = 0.0f, output = 0.0f;
	};

	extern GoldenSearchResult minimizing_golden_search(const std::function<float(float)>& func, float lower_bound, float upper_bound, float input_error_threshold);
	extern EarlyExitGoldenSearchResult early_exit_minimizing_golden_search(const std::function<float(float)>& func, float lower_bound, float upper_bound,
		float input_error_threshold, float early_exit_inclusive_minimum);
	extern GoldenSearchResult maximizing_golden_search(const std::function<float(float)>& func, float lower_bound, float upper_bound, float input_error_threshold);
	extern EarlyExitGoldenSearchResult early_exit_maximizing_golden_search(const std::function<float(float)>& func, float lower_bound, float upper_bound,
		float input_error_threshold, float early_exit_inclusive_maximum);
}
