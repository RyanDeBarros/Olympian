#pragma once

#include "core/util/Time.h"

#include <vector>
#include <unordered_set>
#include <functional>

namespace oly
{
	namespace internal
	{
		class TimerRegistry;
	}

	enum class TimeMode : char
	{
		PROCESSED,
		REAL
	};

	class StateTimer
	{
		friend class internal::TimerRegistry;

		std::vector<float> cumulative_intervals;
		float total_length = 0.0f;
		mutable float elapsed = 0.0f;
		bool one_shot = false;
		mutable bool playing = true;
		TimeMode mode = TimeMode::PROCESSED;
		mutable GLuint _state = 0;

	public:
		StateTimer(const std::vector<float>& intervals, bool one_shot = false, bool playing = true, TimeMode mode = TimeMode::PROCESSED);
		StateTimer(std::vector<float>&& intervals, bool one_shot = false, bool playing = true, TimeMode mode = TimeMode::PROCESSED);
		~StateTimer();
		
		float elapsed_time() const { return elapsed; }
		void pause() { playing = false; }
		void resume() { playing = true; }
		bool is_playing() const { return playing; }

	private:
		void poll() const;

	public:
		struct State
		{
			GLuint index;
			bool first_frame;
		};

		State state() const;
	};

	// TODO v5 getters/setters for data members, especially callback
	class CallbackTimer
	{
		friend class internal::TimerRegistry;

	public:
		using Callback = std::function<void(GLuint)>;

	private:
		std::function<void(GLuint)> callback;
		std::vector<float> cumulative_intervals;
		float total_length = 0.0f;
		bool one_shot = false;
		mutable bool playing = true;
		bool continuous;
		mutable float elapsed = 0.0f;
		TimeMode mode = TimeMode::PROCESSED;
		mutable GLuint _state = 0;

	public:
		CallbackTimer(const std::vector<float>& intervals, const Callback& callback, bool one_shot = false, bool playing = true, bool continuous = true, TimeMode mode = TimeMode::PROCESSED);
		CallbackTimer(std::vector<float>&& intervals, Callback&& callback, bool one_shot = false, bool playing = true, bool continuous = true, TimeMode mode = TimeMode::PROCESSED);
		~CallbackTimer();

		float elapsed_time() const { return elapsed; }
		void pause() { playing = false; }
		void resume() { playing = true; }
		bool is_playing() const { return playing; }

	private:
		void poll() const;
	};

	namespace internal
	{
		class TimerRegistry
		{
			friend class StateTimer;
			std::unordered_set<const StateTimer*> state_timers;
			friend class CallbackTimer;
			std::unordered_set<const CallbackTimer*> callback_timers;

			TimerRegistry() = default;
			TimerRegistry(const TimerRegistry&) = delete;
			TimerRegistry(TimerRegistry&&) = delete;
			~TimerRegistry() = default;

		public:
			static TimerRegistry& instance()
			{
				static TimerRegistry registry;
				return registry;
			}

			void poll_all() const;
		};
	}
}
