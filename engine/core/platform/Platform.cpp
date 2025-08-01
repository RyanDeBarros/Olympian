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
			reg::parse_vec(toml_window_hint["clear color"].as_array(), window_hint.context.clear_color);
			window_hint.context.swap_interval      = (int)toml_window_hint["swap interval"].value<int64_t>().value_or(1);
			window_hint.window.resizable                = toml_window_hint["resizable"].value<bool>().value_or(true);
			window_hint.window.visible                  = toml_window_hint["visible"].value<bool>().value_or(true);
			window_hint.window.decorated                = toml_window_hint["decorated"].value<bool>().value_or(true);
			window_hint.window.focused                  = toml_window_hint["focused"].value<bool>().value_or(true);
			window_hint.window.auto_iconify             = toml_window_hint["auto iconify"].value<bool>().value_or(true);
			window_hint.window.floating                 = toml_window_hint["floating"].value<bool>().value_or(false);
			window_hint.window.maximized                = toml_window_hint["maximized"].value<bool>().value_or(false);
			window_hint.window.center_cursor            = toml_window_hint["center cursor"].value<bool>().value_or(true);
			window_hint.window.transparent_framebuffer  = toml_window_hint["transparent framebuffer"].value<bool>().value_or(false);
			window_hint.window.focus_on_show            = toml_window_hint["focus on show"].value<bool>().value_or(true);
			window_hint.window.scale_to_monitor         = toml_window_hint["scale to monitor"].value<bool>().value_or(false);
			window_hint.window.scale_framebuffer        = toml_window_hint["scale framebuffer"].value<bool>().value_or(true);
			window_hint.window.mouse_passthrough        = toml_window_hint["mouse passthrough"].value<bool>().value_or(false);
			window_hint.window.position_x = (unsigned int)toml_window_hint["position x"].value<int64_t>().value_or(GLFW_ANY_POSITION);
			window_hint.window.position_y = (unsigned int)toml_window_hint["position y"].value<int64_t>().value_or(GLFW_ANY_POSITION);
			window_hint.window.refresh_rate        = (int)toml_window_hint["refresh rate"].value<int64_t>().value_or(GLFW_DONT_CARE);
			window_hint.window.stereo                   = toml_window_hint["stereo"].value<bool>().value_or(false);
			window_hint.window.srgb_capable             = toml_window_hint["srgb capable"].value<bool>().value_or(false);
			window_hint.window.double_buffer            = toml_window_hint["double buffer"].value<bool>().value_or(true);
			window_hint.window.opengl_forward_compat    = toml_window_hint["opengl forward_compat"].value<bool>().value_or(false);
			window_hint.window.context_debug            = toml_window_hint["context debug"].value<bool>().value_or(false);
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
