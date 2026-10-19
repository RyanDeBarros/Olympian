#pragma once

#include <imp/soft_singleton.hpp>

#include <array>
#include <functional>
#include <unordered_set>

namespace oly
{
    enum class TickPhase : char
    {
        None = 0,
        PreFrame,
        TimerPoll,
        Collision,
        Physics,
        Logic,
        PostFrame,
        _c
    };

    enum class TerminatePhase : char
    {
        None = 0,
        Vault,
        Logic,
        Graphics,
        Platform,
        Resources,
        ReferencePool,
        Finalization,
        _c
    };

    struct ITickService;

    namespace context::internal
    {
        class TickServiceRegistry final
            : public imp::soft_singleton<TickServiceRegistry>
        {
            friend class imp::soft_singleton<TickServiceRegistry>;
            friend struct ITickService;

        private:
            std::array<std::unordered_set<ITickService*>, (size_t)TickPhase::_c> tick_services_;
            std::array<std::unordered_set<ITickService*>, (size_t)TerminatePhase::_c> terminate_services_;

        public:
            void tick();
            void terminate();
        };
    }

    struct ITickService
    {
    private:
        TickPhase tick_phase_;
        TerminatePhase terminate_phase_;

    public:
        bool auto_tick = true;

        explicit ITickService(
            TickPhase tick_phase = TickPhase::Logic,
            TerminatePhase terminate_phase = TerminatePhase::None);
        
        ITickService(const ITickService&);
        ITickService(ITickService&&) noexcept;
        ~ITickService();

        virtual void on_tick() {}
        virtual void on_terminate() {}
    };

    struct GenericTickService final
        : public ITickService
    {
        std::function<void()> tick = []() {};
        std::function<void()> terminate = []() {};

        GenericTickService(
            TickPhase tick_phase,
            std::function<void()> tick,
            TerminatePhase terminate_phase,
            std::function<void()> terminate);

        GenericTickService(TickPhase tick_phase, std::function<void()> tick);
        GenericTickService(TerminatePhase terminate_phase, std::function<void()> terminate);
        GenericTickService(std::function<void()> tick, std::function<void()> terminate);

        void on_tick() override;
        void on_terminate() override;
    };

    template<TickPhase OnTickPhase, typename OnTick, TerminatePhase OnTerminatePhase, typename OnTerminate>
    class SingletonTickService final
        : public imp::soft_singleton<SingletonTickService<OnTickPhase, OnTick, OnTerminatePhase, OnTerminate>>
        , public ITickService
    {
        friend class imp::soft_singleton<SingletonTickService<OnTickPhase, OnTick, OnTerminatePhase, OnTerminate>>;

    private:
        SingletonTickService()
            : ITickService(OnTickPhase, OnTerminatePhase)
        {
        }

    public:
        void on_tick() override
        {
            if constexpr (!std::is_void_v<OnTick>)
                OnTick{}();
        }

        void on_terminate() override
        {
            if constexpr (!std::is_void_v<OnTerminate>)
                OnTerminate{}();
        }
    };
}
