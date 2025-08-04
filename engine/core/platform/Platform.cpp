#include "Platform.h"

#include "core/base/Assert.h"

namespace oly::platform
{
	std::unique_ptr<Platform> internal::create_platform(const PlatformSetup& setup)
	{
		return std::unique_ptr<Platform>(new Platform(setup));
	}

	Platform::Platform(const PlatformSetup& setup)
		: _window(setup.window_width, setup.window_height, setup.window_title.c_str(), setup.window_hint), _num_gamepads(setup.num_gamepads), _gamepads(setup.num_gamepads, 0), _binding_context(setup.num_gamepads)
	{
		for (int i = 0; i < _num_gamepads; ++i)
			_gamepads[i] = Gamepad(GLFW_JOYSTICK_1 + i);

		_binding_context.attach_key(&_window.handlers.key);
		_binding_context.attach_mouse_button(&_window.handlers.mouse_button);
		_binding_context.attach_cursor_pos(&_window.handlers.cursor_pos);
		_binding_context.attach_scroll(&_window.handlers.scroll);
	}

	bool Platform::frame()
	{
		_window.swap_buffers();
		oly::internal::check_errors();
		LOG.flush();
		glfwPollEvents();
		_binding_context.poll();
		glClear(per_frame_clear_mask);
		return !_window.should_close();
	}
}
