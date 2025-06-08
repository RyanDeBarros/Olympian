#pragma once

#include "core/util/Time.h"

#include <vector>
#include <functional>

namespace oly
{
	class StateTimer
	{
	public:
		struct State
		{
			size_t index;
			bool first_frame;
		};

	private:
		std::vector<float> cumulative_intervals;
		float total_length;
		float starting_time = 0.0f;
		mutable size_t _state = 0;

	public:
		StateTimer(const std::vector<float>& intervals)
		{
			starting_time = TIME.now<float>();
			cumulative_intervals.reserve(intervals.size());
			total_length = 0.0f;
			for (float interval : intervals)
			{
				total_length += interval;
				cumulative_intervals.push_back(total_length);
			}
		}

		StateTimer(std::vector<float>&& intervals)
			: cumulative_intervals(std::move(intervals))
		{
			starting_time = TIME.now<float>();
			total_length = 0.0f;
			for (size_t i = 0; i < cumulative_intervals.size(); ++i)
			{
				total_length += cumulative_intervals[i];
				cumulative_intervals[i] = total_length;
			}
		}

		float elapsed_time() const { return TIME.now<float>() - starting_time; }
		State state() const
		{
			float local_time = fmod(elapsed_time(), total_length);
			if (local_time < cumulative_intervals[_state])
			{
				if (_state == 0 || local_time >= cumulative_intervals[_state - 1]) // still at _state
					return { _state, false };
				else
				{
					for (size_t i = 0; i < _state; ++i) // earlier than _state
					{
						if (local_time < cumulative_intervals[i])
						{
							_state = i;
							return { _state, true };
						}
					}
				}
			}
			else // later than _state
			{
				for (size_t i = _state + 1; i < cumulative_intervals.size(); ++i)
				{
					if (local_time < cumulative_intervals[i])
					{
						_state = i;
						return { _state, true };
					}
				}
			}
			return { size_t(-1), false}; // should be unreachable
		}
	};

	class CallbackStateTimer
	{
		std::function<void(size_t)> callback;
		std::vector<float> cumulative_intervals;
		float total_length;
		float starting_time = 0.0f;
		mutable size_t _state = 0;
		bool continuous;

	public:
		CallbackStateTimer(const std::vector<float>& intervals, const std::function<void(size_t)>& callback, bool continuous)
			: callback(callback), continuous(continuous)
		{
			starting_time = TIME.now<float>();
			cumulative_intervals.reserve(intervals.size());
			total_length = 0.0f;
			for (float interval : intervals)
			{
				total_length += interval;
				cumulative_intervals.push_back(total_length);
			}
		}

		CallbackStateTimer(std::vector<float>&& intervals, std::function<void(size_t)>&& callback, bool continuous)
			: callback(std::move(callback)), cumulative_intervals(std::move(intervals)), continuous(continuous)
		{
			starting_time = TIME.now<float>();
			total_length = 0.0f;
			for (size_t i = 0; i < cumulative_intervals.size(); ++i)
			{
				total_length += cumulative_intervals[i];
				cumulative_intervals[i] = total_length;
			}
		}

		float elapsed_time() const { return TIME.now<float>() - starting_time; }
		void poll() const
		{
			float local_time = fmod(elapsed_time(), total_length);
			if (local_time < cumulative_intervals[_state])
			{
				if (_state != 0 && local_time < cumulative_intervals[_state - 1]) // earlier than _state
				{
					if (continuous)
					{
						for (size_t i = _state; i < cumulative_intervals.size(); ++i)
							callback(i);

						for (size_t i = 0; i < _state; ++i)
						{
							callback(i);
							if (local_time < cumulative_intervals[i])
							{
								_state = i;
								return;
							}
						}
					}
					else
					{
						for (size_t i = 0; i < _state; ++i)
						{
							if (local_time < cumulative_intervals[i])
							{
								callback(i);
								_state = i;
								return;
							}
						}
					}
				}
			}
			else // later than _state
			{
				if (continuous)
				{
					for (size_t i = _state + 1; i < cumulative_intervals.size(); ++i)
					{
						callback(i);
						if (local_time < cumulative_intervals[i])
						{
							_state = i;
							return;
						}
					}
				}
				else
				{
					for (size_t i = _state + 1; i < cumulative_intervals.size(); ++i)
					{
						if (local_time < cumulative_intervals[i])
						{
							callback(i);
							_state = i;
							return;
						}
					}
				}
			}
		}
	};


}
