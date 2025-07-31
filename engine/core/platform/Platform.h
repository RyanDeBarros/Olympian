#pragma once

#include <memory>

#include "external/TOML.h"
#include "core/platform/Window.h"
#include "core/platform/Gamepad.h"
#include "core/platform/BindingContext.h"

namespace oly::platform
{
	class PlatformSetup
	{
		friend class Platform;

		WindowHint window_hint;
		int window_width, window_height;
		std::string window_title;

		int num_gamepads;

	public:
		PlatformSetup(const TOMLNode& node);

		glm::ivec2 window_size() const { return { window_width, window_height }; }
	};

	class Platform
	{
		Window _window;
		FixedVector<Gamepad> _gamepads;
		int _num_gamepads = 0;
		input::SignalTable _signal_table;
		InputBindingContext _binding_context;

	public:
		Platform(const PlatformSetup& setup);
		Platform(const Platform&) = delete;

		GLenum per_frame_clear_mask = GL_COLOR_BUFFER_BIT;
		bool frame();

		const Window& window() const { return _window; }
		Window& window() { return _window; }
		const Gamepad& gamepad(int i = 0) const { return _gamepads[i]; }
		Gamepad& gamepad(int i = 0) { return _gamepads[i]; }
		const input::SignalTable& signal_table() const { return _signal_table; }
		input::SignalTable& signal_table() { return _signal_table; }
		const InputBindingContext& binding_context() const { return _binding_context; }
		InputBindingContext& binding_context() { return _binding_context; }

		template<std::derived_from<InputController> Controller>
		void bind_signal(const char* signal, bool(Controller::* handler)(input::Signal), const SoftReference<Controller>& controller)
		{
			_binding_context.bind(_signal_table.get(signal), static_cast<InputController::Handler>(handler), controller);
		}

		template<std::derived_from<InputController> Controller>
		void bind_signal(const char* signal, bool(Controller::* handler)(input::Signal) const, const ConstSoftReference<Controller>& controller)
		{
			_binding_context.bind(_signal_table.get(signal), static_cast<InputController::ConstHandler>(handler), controller);
		}

		template<std::derived_from<InputController> Controller>
		void bind_signal(const char* signal, bool(Controller::* handler)(input::Signal), Controller& controller)
		{
			_binding_context.bind(_signal_table.get(signal), static_cast<InputController::Handler>(handler), controller.ref());
		}

		template<std::derived_from<InputController> Controller>
		void bind_signal(const char* signal, bool(Controller::* handler)(input::Signal) const, const Controller& controller)
		{
			_binding_context.bind(_signal_table.get(signal), static_cast<InputController::ConstHandler>(handler), controller.cref());
		}

		template<std::derived_from<InputController> Controller>
		void unbind_signal(const char* signal, bool(Controller::* handler)(input::Signal), const SoftReference<Controller>& controller)
		{
			_binding_context.unbind(_signal_table.get(signal), static_cast<InputController::Handler>(handler), controller);
		}

		template<std::derived_from<InputController> Controller>
		void unbind_signal(const char* signal, bool(Controller::* handler)(input::Signal) const, const ConstSoftReference<Controller>& controller)
		{
			_binding_context.unbind(_signal_table.get(signal), static_cast<InputController::ConstHandler>(handler), controller);
		}

		template<std::derived_from<InputController> Controller>
		void unbind_signal(const char* signal, bool(Controller::* handler)(input::Signal), Controller& controller)
		{
			_binding_context.unbind(_signal_table.get(signal), static_cast<InputController::Handler>(handler), controller.ref());
		}

		template<std::derived_from<InputController> Controller>
		void unbind_signal(const char* signal, bool(Controller::* handler)(input::Signal) const, const Controller& controller)
		{
			_binding_context.unbind(_signal_table.get(signal), static_cast<InputController::ConstHandler>(handler), controller.cref());
		}

		void unbind_signal(const char* signal)
		{
			_binding_context.unbind(_signal_table.get(signal));
		}
	};
}
