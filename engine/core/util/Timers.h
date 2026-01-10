#pragma once

#include "core/util/Time.h"
#include "core/context/TickService.h"

#include <vector>
#include <unordered_set>
#include <functional>

namespace oly
{
	enum class TimeMode : char
	{
		PROCESSED,
		REAL
	};

	class StateTimer : public ITickService
	{
		std::vector<float> cumulative_intervals;
		float total_length = 0.0f;
		mutable float elapsed = 0.0f;
		bool one_shot = false;
		mutable bool playing = true;
		TimeMode mode = TimeMode::PROCESSED;
		mutable GLuint _state = 0;

	public:
		StateTimer(float interval, bool one_shot = false, bool playing = true, TimeMode mode = TimeMode::PROCESSED);
		StateTimer(const std::vector<float>& intervals, bool one_shot = false, bool playing = true, TimeMode mode = TimeMode::PROCESSED);
		StateTimer(std::vector<float>&& intervals, bool one_shot = false, bool playing = true, TimeMode mode = TimeMode::PROCESSED);
		
		float elapsed_time() const { return elapsed; }
		void pause() { playing = false; }
		void resume() { playing = true; }
		bool is_playing() const { return playing; }

	private:
		void poll() const;

	public:
		void on_tick() override { poll(); }

		struct State
		{
			GLuint index;
			bool first_frame;
		};

		State state() const;
	};

	class CallbackTimer : public ITickService
	{
	public:
		using Callback = std::function<void(GLuint)>;

		std::function<void(GLuint)> callback;

	private:
		std::vector<float> cumulative_intervals;
		float total_length = 0.0f;
		mutable float elapsed = 0.0f;
		bool one_shot = false;
		mutable bool playing = true;
		bool continuous;
		TimeMode mode = TimeMode::PROCESSED;
		mutable GLuint _state = 0;

	public:
		CallbackTimer(float interval, const Callback& callback = {}, bool one_shot = false, bool playing = true, bool continuous = true, TimeMode mode = TimeMode::PROCESSED);
		CallbackTimer(const std::vector<float>& intervals, const Callback& callback, bool one_shot = false, bool playing = true, bool continuous = true, TimeMode mode = TimeMode::PROCESSED);
		CallbackTimer(std::vector<float>&& intervals, Callback&& callback, bool one_shot = false, bool playing = true, bool continuous = true, TimeMode mode = TimeMode::PROCESSED);

		float elapsed_time() const { return elapsed; }
		void pause() { playing = false; }
		void resume() { playing = true; }
		bool is_playing() const { return playing; }

	private:
		void poll() const;

	public:
		void on_tick() override { poll(); }
	};
}
