#pragma once

#include "core/platform/Signal.h"

namespace oly
{
	namespace input::internal
	{
		class InputBindingContext;
	}

	struct InputController
	{
		InputController();
		InputController(const InputController&) = delete;
		InputController(InputController&&) noexcept = delete;
		virtual ~InputController();

	private:
		void insert_signal(input::SignalID) const;
		void erase_signal(input::SignalID) const;

	public:
		using Handler = bool(InputController::*)(input::Signal);
		using ConstHandler = bool(InputController::*)(input::Signal) const;

		void bind(input::SignalID signal, Handler handler);
		void bind(input::SignalID signal, ConstHandler handler) const;
		void unbind(input::SignalID signal) const;

		void bind(const StringParam& signal, Handler handler);
		void bind(const StringParam& signal, ConstHandler handler) const;
		void unbind(const StringParam& signal) const;

		template<std::derived_from<InputController> Controller>
		void bind(const StringParam& signal, bool(Controller::* handler)(input::Signal)) { bind(signal, static_cast<Handler>(handler)); }
		template<std::derived_from<InputController> Controller>
		void bind(const StringParam& signal, bool(Controller::* handler)(input::Signal) const) const { bind(signal, static_cast<ConstHandler>(handler)); }

		void bind_mapping(const StringParam& mapping, Handler handler);
		void bind_mapping(const StringParam& mapping, ConstHandler handler) const;
		void unbind_mapping(const StringParam& mapping) const;

		template<std::derived_from<InputController> Controller>
		void bind_mapping(const StringParam& mapping, bool(Controller::* handler)(input::Signal)) { bind_mapping(mapping, static_cast<Handler>(handler)); }
		template<std::derived_from<InputController> Controller>
		void bind_mapping(const StringParam& mapping, bool(Controller::* handler)(input::Signal) const) const { bind_mapping(mapping, static_cast<ConstHandler>(handler)); }
	};
}
