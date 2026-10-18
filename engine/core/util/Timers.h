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
        Processed,
        Real
    };

    class StateTimer
        : public ITickService
    {
    public:
        struct State
        {
            GLuint index;
            bool first_frame;
        };

    private:
        std::vector<float> cumulative_intervals_;
        float total_length_ = 0.0f;
        mutable float elapsed_ = 0.0f;
        bool one_shot_ = false;
        mutable bool playing_ = true;
        TimeMode mode_ = TimeMode::Processed;
        mutable GLuint state_ = 0;

    public:
        StateTimer(
            float interval,
            bool one_shot = false,
            bool playing = true,
            TimeMode mode = TimeMode::Processed);

        StateTimer(
            std::vector<float> intervals,
            bool one_shot = false,
            bool playing = true,
            TimeMode mode = TimeMode::Processed);

    public:
        void on_tick() override;

        float elapsed_time() const;
        void pause();
        void resume();
        bool is_playing() const;

        State state() const;

    private:
        void poll() const;
    };

    class CallbackTimer
        : public ITickService
    {
    public:
        using Callback = std::function<void(GLuint)>;

    public:
        std::function<void(GLuint)> callback;

    private:
        std::vector<float> cumulative_intervals_;
        float total_length_ = 0.0f;
        mutable float elapsed_ = 0.0f;
        bool one_shot_ = false;
        mutable bool playing_ = true;
        bool continuous_;
        TimeMode mode_ = TimeMode::Processed;
        mutable GLuint state_ = 0;

    public:
        CallbackTimer(
            float interval,
            Callback callback = {},
            bool one_shot = false,
            bool playing = true,
            bool continuous = true,
            TimeMode mode = TimeMode::Processed);

        CallbackTimer(
            std::vector<float> intervals,
            Callback callback,
            bool one_shot = false,
            bool playing = true,
            bool continuous = true,
            TimeMode mode = TimeMode::Processed);

    public:
        void on_tick() override;

        float elapsed_time() const;
        void pause();
        void resume();
        bool is_playing() const;

    private:
        void poll() const;
    };
}
