#include "GoldenSectionSearch.h"

namespace oly::algo
{
	static float split_left(float a, float b)
	{
		return b - (b - a) * inv_golden_ratio();
	}

	static float split_right(float a, float b)
	{
		return a + (b - a) * inv_golden_ratio();
	}

	struct Quadruple
	{
		float outer_left = 0.0f, inner_left = 0.0f, inner_right = 0.0f, outer_right = 0.0f;
	};

	static size_t golden_iterations(float lower_bound, float upper_bound, float input_error_threshold)
	{
		return (size_t)roundf(glm::log(input_error_threshold / (upper_bound - lower_bound)) * inv_log_inv_golden_ratio());
	}

	static void select_left(const std::function<float(float)>& func, Quadruple& inputs, Quadruple& outputs)
	{
		inputs.outer_right = inputs.inner_right;
		outputs.outer_right = outputs.inner_right;

		inputs.inner_right = inputs.inner_left;
		outputs.inner_right = outputs.inner_left;

		inputs.inner_left = split_left(inputs.outer_left, inputs.outer_right);
		outputs.inner_left = func(inputs.inner_left);
	}

	static void select_right(const std::function<float(float)>& func, Quadruple& inputs, Quadruple& outputs)
	{
		inputs.outer_left = inputs.inner_left;
		outputs.outer_left = outputs.inner_left;

		inputs.inner_left = inputs.inner_right;
		outputs.inner_left = outputs.inner_right;

		inputs.inner_right = split_right(inputs.outer_left, inputs.outer_right);
		outputs.inner_right = func(inputs.inner_right);
	}
	
	template<typename Comparator>
	static GoldenSearchResult golden_search_result(const std::function<float(float)>& func, Quadruple inputs, Quadruple outputs, Comparator comparator)
	{
		if (comparator(outputs.inner_left, outputs.inner_right))
		{
			if (comparator(outputs.outer_left, outputs.inner_left))
				return GoldenSearchResult{ .input = inputs.outer_left, .output = outputs.outer_left };
			else if (comparator(outputs.inner_left, outputs.outer_left))
				return GoldenSearchResult{ .input = inputs.inner_left, .output = outputs.inner_left };
			else
			{
				GoldenSearchResult result{ .input = 0.5f * (inputs.outer_left + inputs.inner_left) };
				result.output = func(result.input);
				return result;
			}
		}
		else if (comparator(outputs.inner_right, outputs.inner_left))
		{
			if (comparator(outputs.outer_right, outputs.inner_right))
				return GoldenSearchResult{ .input = inputs.outer_right, .output = outputs.outer_right };
			else if (comparator(outputs.inner_right, outputs.outer_right))
				return GoldenSearchResult{ .input = inputs.inner_right, .output = outputs.inner_right };
			else
			{
				GoldenSearchResult result{ .input = 0.5f * (inputs.outer_right + inputs.inner_right) };
				result.output = func(result.input);
				return result;
			}
		}
		else
		{
			GoldenSearchResult result{ .input = 0.5f * (inputs.inner_left + inputs.inner_right) };
			result.output = func(result.input);
			return result;
		}
	}

	template<typename Comparator>
	GoldenSearchResult generic_golden_search(const std::function<float(float)>& func, float lower_bound, float upper_bound, float input_error_threshold, Comparator comparator)
	{
		Quadruple inputs{ .outer_left = lower_bound, .inner_left = split_left(lower_bound, upper_bound), .inner_right = split_right(lower_bound, upper_bound), .outer_right = upper_bound };
		Quadruple outputs{ .outer_left = func(inputs.outer_left), .inner_left = func(inputs.inner_left), .inner_right = func(inputs.inner_right), .outer_right = func(inputs.outer_right) };
		
		const size_t iterations = golden_iterations(lower_bound, upper_bound, input_error_threshold);
		for (size_t _ = 0; _ < iterations; ++_)
		{
			if (comparator(outputs.inner_left, outputs.inner_right))
				select_left(func, inputs, outputs);
			else
				select_right(func, inputs, outputs);
		}

		return golden_search_result(func, inputs, outputs, comparator);
	}

	template<typename Comparator, typename InclusiveComparator>
	EarlyExitGoldenSearchResult generic_golden_search(const std::function<float(float)>& func, float lower_bound, float upper_bound,
		float input_error_threshold, float early_exit_inclusive_extremum, Comparator comparator, InclusiveComparator inclusive_comparator)
	{
		Quadruple inputs{ .outer_left = lower_bound, .inner_left = split_left(lower_bound, upper_bound), .inner_right = split_right(lower_bound, upper_bound), .outer_right = upper_bound };
		Quadruple outputs;
		outputs.outer_left = func(inputs.outer_left);
		if (inclusive_comparator(outputs.outer_left, early_exit_inclusive_extremum))
			return EarlyExitGoldenSearchResult{ .early_exited = true };
		outputs.inner_left = func(inputs.inner_left);
		if (inclusive_comparator(outputs.inner_left, early_exit_inclusive_extremum))
			return EarlyExitGoldenSearchResult{ .early_exited = true };
		outputs.inner_right = func(inputs.inner_right);
		if (inclusive_comparator(outputs.inner_right, early_exit_inclusive_extremum))
			return EarlyExitGoldenSearchResult{ .early_exited = true };
		outputs.outer_right = func(inputs.outer_right);
		if (inclusive_comparator(outputs.outer_right, early_exit_inclusive_extremum))
			return EarlyExitGoldenSearchResult{ .early_exited = true };
		
		const size_t iterations = golden_iterations(lower_bound, upper_bound, input_error_threshold);
		for (size_t _ = 0; _ < iterations; ++_)
		{
			if (comparator(outputs.inner_left, outputs.inner_right))
			{
				select_left(func, inputs, outputs);
				if (inclusive_comparator(outputs.inner_left, early_exit_inclusive_extremum))
					return EarlyExitGoldenSearchResult{ .early_exited = true };
			}
			else
			{
				select_right(func, inputs, outputs);
				if (inclusive_comparator(outputs.inner_right, early_exit_inclusive_extremum))
					return EarlyExitGoldenSearchResult{ .early_exited = true };
			}
		}

		GoldenSearchResult result = golden_search_result(func, inputs, outputs, comparator);
		return inclusive_comparator(result.output, early_exit_inclusive_extremum) ? EarlyExitGoldenSearchResult{ .early_exited = true }
			: EarlyExitGoldenSearchResult{ .early_exited = false, .input = result.input, .output = result.output };
	}

	GoldenSearchResult minimizing_golden_search(const std::function<float(float)>& func, float lower_bound, float upper_bound, float input_error_threshold)
	{
		return generic_golden_search(func, lower_bound, upper_bound, input_error_threshold, [](float a, float b) { return a < b; });
	}

	EarlyExitGoldenSearchResult early_exit_minimizing_golden_search(const std::function<float(float)>& func, float lower_bound, float upper_bound,
		float input_error_threshold, float early_exit_inclusive_minimum)
	{
		return generic_golden_search(func, lower_bound, upper_bound, input_error_threshold, early_exit_inclusive_minimum,
			[](float a, float b) { return a < b; }, [](float a, float b) { return a <= b; });
	}

	GoldenSearchResult maximizing_golden_search(const std::function<float(float)>& func, float lower_bound, float upper_bound, float input_error_threshold)
	{
		return generic_golden_search(func, lower_bound, upper_bound, input_error_threshold, [](float a, float b) { return a > b; });
	}

	EarlyExitGoldenSearchResult early_exit_maximizing_golden_search(const std::function<float(float)>& func, float lower_bound, float upper_bound,
		float input_error_threshold, float early_exit_inclusive_maximum)
	{
		return generic_golden_search(func, lower_bound, upper_bound, input_error_threshold, early_exit_inclusive_maximum,
			[](float a, float b) { return a > b; }, [](float a, float b) { return a >= b; });
	}
}
