#include "Timers.h"

namespace oly
{
	static void init_intervals(std::vector<float>& cumulative_intervals, float& total_length)
	{
		if (cumulative_intervals.size() > 1)
		{
			for (size_t i = 0; i < cumulative_intervals.size(); ++i)
			{
				total_length += cumulative_intervals[i];
				cumulative_intervals[i] = total_length;
			}
		}
		else if (cumulative_intervals.size() == 1)
		{
			cumulative_intervals = { cumulative_intervals[0], 2.0f * cumulative_intervals[0] };
			total_length = cumulative_intervals[1];
		}
		else
		{
			total_length = 0.0f;
			cumulative_intervals = { 0.0f };
		}
	}

	StateTimer::StateTimer(const std::vector<float>& intervals, bool one_shot, bool playing)
		: cumulative_intervals(intervals), one_shot(one_shot), playing(playing)
	{
		init_intervals(cumulative_intervals, total_length);
		internal::TimerRegistry::instance().state_timers.insert(this);
	}

	StateTimer::StateTimer(std::vector<float>&& intervals, bool one_shot, bool playing)
		: cumulative_intervals(std::move(intervals)), one_shot(one_shot), playing(playing)
	{
		init_intervals(cumulative_intervals, total_length);
		internal::TimerRegistry::instance().state_timers.insert(this);
	}

	StateTimer::~StateTimer()
	{
		internal::TimerRegistry::instance().state_timers.erase(this);
	}

	void StateTimer::poll() const
	{
		if (playing)
		{
			elapsed += TIME.delta();
			if (one_shot && elapsed >= total_length)
				playing = false;
		}
	}

	StateTimer::State StateTimer::state() const
	{
		float local_time = fmod(elapsed, total_length);
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
		return { size_t(-1), false }; // should be unreachable
	}

	CallbackTimer::CallbackTimer(const std::vector<float>& intervals, const std::function<void(size_t)>& callback, bool one_shot, bool playing, bool continuous)
		: callback(callback), cumulative_intervals(intervals), one_shot(one_shot), playing(playing), continuous(continuous)
	{
		init_intervals(cumulative_intervals, total_length);
		internal::TimerRegistry::instance().callback_timers.insert(this);
	}

	CallbackTimer::CallbackTimer(std::vector<float>&& intervals, std::function<void(size_t)>&& callback, bool one_shot, bool playing, bool continuous)
		: callback(std::move(callback)), cumulative_intervals(std::move(intervals)), one_shot(one_shot), playing(playing), continuous(continuous)
	{
		init_intervals(cumulative_intervals, total_length);
		internal::TimerRegistry::instance().callback_timers.insert(this);
	}

	CallbackTimer::~CallbackTimer()
	{
		internal::TimerRegistry::instance().callback_timers.erase(this);
	}

	void CallbackTimer::poll() const
	{
		if (!playing)
			return;

		elapsed += TIME.delta();
		if (one_shot && elapsed >= total_length)
		{
			playing = false;
			return;
		}

		const float local_time = fmod(elapsed, total_length);
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

	void internal::TimerRegistry::poll_all() const
	{
		for (const StateTimer* timer : state_timers)
			timer->poll();
		for (const CallbackTimer* timer : callback_timers)
			timer->poll();
	}
}
