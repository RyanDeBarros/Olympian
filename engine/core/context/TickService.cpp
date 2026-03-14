#include "TickService.h"

namespace oly
{
	void context::internal::TickServiceRegistry::tick()
	{
		for (auto& service_set : tick_services)
			for (ITickService* service : service_set)
				if (service->auto_tick) [[likely]]
					service->on_tick();
	}

	void context::internal::TickServiceRegistry::terminate()
	{
		for (auto& service_set : terminate_services)
			for (ITickService* service : service_set)
				service->on_terminate();

		for (auto& service_set : tick_services)
			service_set.clear();

		for (auto& service_set : terminate_services)
			service_set.clear();
	}

	ITickService::ITickService(TickPhase tick_phase, TerminatePhase terminate_phase)
		: tick_phase(tick_phase), terminate_phase(terminate_phase)
	{
		if (tick_phase != TickPhase::None)
			context::internal::TickServiceRegistry::instance().tick_services[(size_t)tick_phase].insert(this);
		if (terminate_phase != TerminatePhase::None)
			context::internal::TickServiceRegistry::instance().terminate_services[(size_t)terminate_phase].insert(this);
	}

	ITickService::ITickService(const ITickService& other)
		: tick_phase(other.tick_phase), terminate_phase(other.terminate_phase)
	{
		if (tick_phase != TickPhase::None)
			context::internal::TickServiceRegistry::instance().tick_services[(size_t)tick_phase].insert(this);
		if (terminate_phase != TerminatePhase::None)
			context::internal::TickServiceRegistry::instance().terminate_services[(size_t)terminate_phase].insert(this);
	}

	ITickService::ITickService(ITickService&& other) noexcept
		: tick_phase(other.tick_phase), terminate_phase(other.terminate_phase)
	{
		if (tick_phase != TickPhase::None)
			context::internal::TickServiceRegistry::instance().tick_services[(size_t)tick_phase].insert(this);
		if (terminate_phase != TerminatePhase::None)
			context::internal::TickServiceRegistry::instance().terminate_services[(size_t)terminate_phase].insert(this);
	}

	ITickService::~ITickService()
	{
		if (tick_phase != TickPhase::None)
			context::internal::TickServiceRegistry::instance().tick_services[(size_t)tick_phase].erase(this);
		if (terminate_phase != TerminatePhase::None)
			context::internal::TickServiceRegistry::instance().terminate_services[(size_t)terminate_phase].erase(this);
	}
}
