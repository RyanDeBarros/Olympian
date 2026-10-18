#include "Timers.h"

#include "core/base/Errors.h"

namespace oly
{
    static void init_intervals(std::vector<float>& cumulative_intervals_, float& total_length)
    {
        if (cumulative_intervals_.size() > 1)
        {
            for (size_t i = 0; i < cumulative_intervals_.size(); ++i)
            {
                total_length += cumulative_intervals_[i];
                cumulative_intervals_[i] = total_length;
            }
        }
        else if (cumulative_intervals_.size() == 1)
        {
            cumulative_intervals_ = { cumulative_intervals_[0], 2.0f * cumulative_intervals_[0] };
            total_length = cumulative_intervals_[1];
        }
        else
        {
            total_length = 0.0f;
            cumulative_intervals_ = { 0.0f };
        }
    }

    static float delta_time(TimeMode mode)
    {
        if (mode == TimeMode::Processed) [[likely]]
            return kTime.delta();
        else
            return kRealTime.delta();
    }

    StateTimer::StateTimer(float interval, bool one_shot, bool playing, TimeMode mode)
        : ITickService(TickPhase::TimerPoll)
        , cumulative_intervals_({ interval })
        , one_shot_(one_shot)
        , playing_(playing)
        , mode_(mode)
    {
        init_intervals(cumulative_intervals_, total_length_);
    }

    StateTimer::StateTimer(std::vector<float> intervals, bool one_shot, bool playing, TimeMode mode)
        : ITickService(TickPhase::TimerPoll)
        , cumulative_intervals_(std::move(intervals))
        , one_shot_(one_shot)
        , playing_(playing)
        , mode_(mode)
    {
        init_intervals(cumulative_intervals_, total_length_);
    }

    void StateTimer::on_tick()
    {
        poll();
    }

    float StateTimer::elapsed_time() const
    {
        return elapsed_;
    }

    void StateTimer::pause()
    {
        playing_ = false;
    }

    void StateTimer::resume()
    {
        playing_ = true;
    }

    bool StateTimer::is_playing() const
    {
        return playing_;
    }

    StateTimer::State StateTimer::state() const
    {
        float local_time = fmod(elapsed_, total_length_);
        if (local_time < cumulative_intervals_[state_])
        {
            if (state_ == 0 || local_time >= cumulative_intervals_[state_ - 1]) // still at state_
                return { state_, false };
            else
            {
                for (size_t i = 0; i < state_; ++i) // earlier than state_
                {
                    if (local_time < cumulative_intervals_[i])
                    {
                        state_ = i;
                        return { state_, true };
                    }
                }
            }
        }
        else // later than state_
        {
            for (size_t i = state_ + 1; i < cumulative_intervals_.size(); ++i)
            {
                if (local_time < cumulative_intervals_[i])
                {
                    state_ = i;
                    return { state_, true };
                }
            }
        }

        throw Error(ErrorCode::UnreachableCode);
    }

    void StateTimer::poll() const
    {
        if (playing_)
        {
            elapsed_ += delta_time(mode_);
            if (one_shot_ && elapsed_ >= total_length_)
                playing_ = false;
        }
    }

    CallbackTimer::CallbackTimer(
        float interval,
        Callback callback,
        bool one_shot,
        bool playing,
        bool continuous,
        TimeMode mode)
        : ITickService(TickPhase::TimerPoll)
        , callback(std::move(callback))
        , cumulative_intervals_({ interval })
        , one_shot_(one_shot)
        , playing_(playing)
        , continuous_(continuous)
        , mode_(mode)
    {
        init_intervals(cumulative_intervals_, total_length_);
    }

    CallbackTimer::CallbackTimer(
        std::vector<float> intervals,
        Callback callback,
        bool one_shot,
        bool playing,
        bool continuous,
        TimeMode mode)
        : ITickService(TickPhase::TimerPoll)
        , callback(std::move(callback))
        , cumulative_intervals_(std::move(intervals))
        , one_shot_(one_shot)
        , playing_(playing)
        , continuous_(continuous)
        , mode_(mode)
    {
        init_intervals(cumulative_intervals_, total_length_);
    }

    void CallbackTimer::on_tick()
    {
        poll();
    }

    float CallbackTimer::elapsed_time() const
    {
        return elapsed_;
    }
    
    void CallbackTimer::pause()
    {
        playing_ = false;
    }
    
    void CallbackTimer::resume()
    {
        playing_ = true;
    }
    
    bool CallbackTimer::is_playing() const
    {
        return playing_;
    }

    void CallbackTimer::poll() const
    {
        if (!playing_)
            return;

        elapsed_ += delta_time(mode_);
        if (one_shot_ && elapsed_ >= total_length_)
        {
            playing_ = false;
            return;
        }

        const float local_time = fmod(elapsed_, total_length_);
        if (local_time < cumulative_intervals_[state_])
        {
            if (state_ != 0 && local_time < cumulative_intervals_[state_ - 1]) // earlier than state_
            {
                if (continuous_)
                {
                    for (GLuint i = state_; i < cumulative_intervals_.size(); ++i)
                        callback(i);

                    for (GLuint i = 0; i < state_; ++i)
                    {
                        callback(i);
                        if (local_time < cumulative_intervals_[i])
                        {
                            state_ = i;
                            return;
                        }
                    }
                }
                else
                {
                    for (GLuint i = 0; i < state_; ++i)
                    {
                        if (local_time < cumulative_intervals_[i])
                        {
                            callback(i);
                            state_ = i;
                            return;
                        }
                    }
                }
            }
        }
        else // later than state_
        {
            if (continuous_)
            {
                for (GLuint i = state_ + 1; i < cumulative_intervals_.size(); ++i)
                {
                    callback(i);
                    if (local_time < cumulative_intervals_[i])
                    {
                        state_ = i;
                        return;
                    }
                }
            }
            else
            {
                for (GLuint i = state_ + 1; i < cumulative_intervals_.size(); ++i)
                {
                    if (local_time < cumulative_intervals_[i])
                    {
                        callback(i);
                        state_ = i;
                        return;
                    }
                }
            }
        }
    }
}
