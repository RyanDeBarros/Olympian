#include "Platform.h"

#include "core/base/Assert.h"
#include "registries/Loader.h"

namespace oly::platform
{
	PlatformSetup::PlatformSetup(const TOMLNode& node)
	{
		auto toml_window = node["window"];
		if (!toml_window)
			throw Error(ErrorCode::PLATFORM_INIT);
		auto width = toml_window["width"].value<int64_t>();
		auto height = toml_window["height"].value<int64_t>();
		auto title = toml_window["title"].value<std::string>();
		if (!width || !height || !title)
			throw Error(ErrorCode::PLATFORM_INIT);

		window_width = (int)width.value();
		window_height = (int)height.value();
		window_title = title.value();

		if (auto toml_window_hint = node["window_hint"])
		{
			reg::parse_vec4(toml_window_hint, "clear color", window_hint.context.clear_color);
			// TODO rest of window hint options
		}

		num_gamepads = glm::clamp((int)node["gamepads"].value<int64_t>().value_or(0), 0, GLFW_JOYSTICK_LAST);
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
		internal::check_errors();
		LOG.flush();
		glfwPollEvents();
		_binding_context.poll();
		glClear(per_frame_clear_mask);
		return !_window.should_close();
	}
}
