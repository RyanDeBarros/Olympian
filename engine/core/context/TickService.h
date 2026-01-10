#pragma once

#include "core/types/AutoRegistry.h"

#include <unordered_set>
#include <array>
#include <functional>

namespace oly
{
	// TODO v6 use PascalCase for 'state' enums
	enum class TickPhase : char
	{
		PreFrame,
		TimerPoll,
		Collision,
		Physics,
		Logic,
		PostFrame,
		None
	};

	enum class TerminatePhase : char
	{
		Vault,
		Logic,
		Graphics,
		Platform,
		Resources,
		ReferencePool,
		Finalization,
		None
	};

	struct ITickService;

	namespace context::internal
	{
		class TickServiceRegistry final : public Singleton<TickServiceRegistry>
		{
			friend class Singleton<TickServiceRegistry>;

			friend struct ITickService;
			std::array<std::unordered_set<ITickService*>, (size_t)TickPhase::None> tick_services;
			std::array<std::unordered_set<ITickService*>, (size_t)TerminatePhase::None> terminate_services;

		public:
			void tick();
			void terminate();
		};
	}

	struct ITickService
	{
	private:
		TickPhase tick_phase;
		TerminatePhase terminate_phase;

	public:
		bool auto_tick = true;

		explicit ITickService(TickPhase tick_phase = TickPhase::Logic, TerminatePhase terminate_phase = TerminatePhase::None);
		ITickService(const ITickService&);
		ITickService(ITickService&&) noexcept;
		~ITickService();

		virtual void on_tick() {}
		virtual void on_terminate() {}
	};

	struct GenericTickService final : public ITickService
	{
		std::function<void()> tick = []() {};
		std::function<void()> terminate = []() {};

		explicit GenericTickService(TickPhase tick_phase, std::function<void()>&& tick, TerminatePhase terminate_phase, std::function<void()>&& terminate)
			: ITickService(tick_phase, terminate_phase), tick(std::move(tick)), terminate(std::move(terminate)) {
		}

		explicit GenericTickService(TickPhase tick_phase, std::function<void()>&& tick)
			: ITickService(tick_phase, TerminatePhase::None), tick(std::move(tick)) {}

		explicit GenericTickService(TerminatePhase terminate_phase, std::function<void()>&& terminate)
			: ITickService(TickPhase::None, terminate_phase), terminate(std::move(terminate)) {}

		explicit GenericTickService(std::function<void()>&& tick, std::function<void()>&& terminate)
			: ITickService(), tick(std::move(tick)), terminate(std::move(terminate)) {
		}

		void on_tick() override { tick(); }
		void on_terminate() override { terminate(); }
	};
}
