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
		InputController(const InputController&);
		InputController(InputController&&) noexcept;
		virtual ~InputController();
		InputController& operator=(const InputController&);
		InputController& operator=(InputController&&) noexcept;

	private:
		void move_controller(InputController&&) const;
		void erase_controller() const;

		void insert_signal(input::SignalID) const;
		void erase_signal(input::SignalID) const;

	public:
		using Handler = bool(InputController::*)(input::Signal);
		using ConstHandler = bool(InputController::*)(input::Signal) const;

		void bind(input::SignalID signal, Handler handler);
		void bind(input::SignalID signal, ConstHandler handler) const;
		void unbind(input::SignalID signal) const;

		void bind(const std::string& signal, Handler handler);
		void bind(const std::string& signal, ConstHandler handler) const;
		void unbind(const std::string& signal) const;

		template<std::derived_from<InputController> Controller>
		void bind(const std::string& signal, bool(Controller::* handler)(input::Signal)) { bind(signal, static_cast<Handler>(handler)); }
		template<std::derived_from<InputController> Controller>
		void bind(const std::string& signal, bool(Controller::* handler)(input::Signal) const) const { bind(signal, static_cast<ConstHandler>(handler)); }

		void bind_mapping(const std::string& mapping, Handler handler);
		void bind_mapping(const std::string& mapping, ConstHandler handler) const;
		void unbind_mapping(const std::string& mapping) const;

		template<std::derived_from<InputController> Controller>
		void bind_mapping(const std::string& mapping, bool(Controller::* handler)(input::Signal)) { bind_mapping(mapping, static_cast<Handler>(handler)); }
		template<std::derived_from<InputController> Controller>
		void bind_mapping(const std::string& mapping, bool(Controller::* handler)(input::Signal) const) const { bind_mapping(mapping, static_cast<ConstHandler>(handler)); }
	};

#define OLY_INPUT_CONTROLLER_HEADER(Class) OLY_SOFT_REFERENCE_PUBLIC(Class)
}
