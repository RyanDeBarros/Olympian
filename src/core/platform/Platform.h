#pragma once

#include <memory>

#include "external/TOML.h"
#include "core/platform/Window.h"

namespace oly
{
	class PlatformSetup
	{
		friend class Platform;

		rendering::WindowHint window_hint;
		int window_width, window_height;
		std::string window_title;

		int num_gamepads;

	public:
		PlatformSetup(const TOMLNode& node);
	};

	class Platform
	{
		rendering::Window _window;
		FixedVector<input::Gamepad> _gamepads;
		int _num_gamepads = 0;
		input::SignalTable _signal_table;
		input::BindingContext _binding_context;

	public:
		Platform(const PlatformSetup& setup);
		Platform(const Platform&) = delete;

		GLenum per_frame_clear_mask = GL_COLOR_BUFFER_BIT;
		bool frame();

		const rendering::Window& window() const { return _window; }
		rendering::Window& window() { return _window; }
		const input::Gamepad& gamepad(int i = 0) const { return _gamepads[i]; }
		input::Gamepad& gamepad(int i = 0) { return _gamepads[i]; }
		const input::SignalTable& signal_table() const { return _signal_table; }
		input::SignalTable& signal_table() { return _signal_table; }
		const input::BindingContext& binding_context() const { return _binding_context; }
		input::BindingContext& binding_context() { return _binding_context; }

		template<std::derived_from<input::InputController> Controller>
		void bind_signal(const char* signal, bool(Controller::* handler)(input::Signal), Controller& controller)
		{
			_binding_context.bind(_signal_table.get(signal), static_cast<input::InputController::Handler>(handler), &controller);
		}

		template<std::derived_from<input::InputController> Controller>
		void unbind_signal(const char* signal, bool(Controller::* handler)(input::Signal), Controller& controller)
		{
			_binding_context.unbind(_signal_table.get(signal), static_cast<input::InputController::Handler>(handler), &controller);
		}
	};
}
