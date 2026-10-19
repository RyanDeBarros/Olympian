#include "TickService.h"

namespace oly
{
    void context::internal::TickServiceRegistry::tick()
    {
        for (auto& service_set : tick_services_)
            for (ITickService* service : service_set)
                if (service->auto_tick) [[likely]]
                    service->on_tick();
    }

    void context::internal::TickServiceRegistry::terminate()
    {
        for (auto& service_set : terminate_services_)
            for (ITickService* service : service_set)
                service->on_terminate();

        for (auto& service_set : tick_services_)
            service_set.clear();

        for (auto& service_set : terminate_services_)
            service_set.clear();
    }

    ITickService::ITickService(TickPhase tick_phase, TerminatePhase terminate_phase)
        : tick_phase_(tick_phase)
        , terminate_phase_(terminate_phase)
    {
        if (tick_phase_ != TickPhase::None)
            context::internal::TickServiceRegistry::instance().tick_services_[(size_t)tick_phase_].insert(this);

        if (terminate_phase_ != TerminatePhase::None)
        {
            context::internal::TickServiceRegistry::instance()
                .terminate_services_[(size_t)terminate_phase_].insert(this);
        }
    }

    ITickService::ITickService(const ITickService& o)
        : tick_phase_(o.tick_phase_)
        , terminate_phase_(o.terminate_phase_)
    {
        if (tick_phase_ != TickPhase::None)
            context::internal::TickServiceRegistry::instance().tick_services_[(size_t)tick_phase_].insert(this);

        if (terminate_phase_ != TerminatePhase::None)
        {
            context::internal::TickServiceRegistry::instance()
                .terminate_services_[(size_t)terminate_phase_].insert(this);
        }
    }

    ITickService::ITickService(ITickService&& o) noexcept
        : tick_phase_(o.tick_phase_)
        , terminate_phase_(o.terminate_phase_)
    {
        if (tick_phase_ != TickPhase::None)
            context::internal::TickServiceRegistry::instance().tick_services_[(size_t)tick_phase_].insert(this);

        if (terminate_phase_ != TerminatePhase::None)
        {
            context::internal::TickServiceRegistry::instance()
                .terminate_services_[(size_t)terminate_phase_].insert(this);
        }
    }

    ITickService::~ITickService()
    {
        if (tick_phase_ != TickPhase::None)
            context::internal::TickServiceRegistry::instance().tick_services_[(size_t)tick_phase_].erase(this);

        if (terminate_phase_ != TerminatePhase::None)
        {
            context::internal::TickServiceRegistry::instance()
                .terminate_services_[(size_t)terminate_phase_].erase(this);
        }
    }

    GenericTickService::GenericTickService(
        TickPhase tick_phase,
        std::function<void()> tick,
        TerminatePhase terminate_phase,
        std::function<void()> terminate)
        : ITickService(tick_phase, terminate_phase)
        , tick(std::move(tick))
        , terminate(std::move(terminate))
    {
    }

    GenericTickService::GenericTickService(TickPhase tick_phase, std::function<void()> tick)
        : ITickService(tick_phase, TerminatePhase::None)
        , tick(std::move(tick))
    {
    }

    GenericTickService::GenericTickService(TerminatePhase terminate_phase, std::function<void()> terminate)
        : ITickService(TickPhase::None, terminate_phase)
        , terminate(std::move(terminate))
    {
    }

    GenericTickService::GenericTickService(std::function<void()> tick, std::function<void()> terminate)
        : ITickService()
        , tick(std::move(tick))
        , terminate(std::move(terminate))
    {
    }

    void GenericTickService::on_tick()
    {
        tick();
    }
    
    void GenericTickService::on_terminate()
    {
        terminate();
    }
}
