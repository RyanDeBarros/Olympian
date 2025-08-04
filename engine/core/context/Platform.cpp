#include "Platform.h"

#include "core/context/Context.h"
#include "core/base/Errors.h"
#include "core/util/Logger.h"
#include "core/algorithms/STLUtils.h"

#include "registries/Loader.h"

namespace oly::context
{
	namespace internal
	{
		std::unique_ptr<platform::Platform> platform;

		glm::ivec2 initial_window_size;

		platform::WRViewport wr_viewport;
		platform::WRDrawer wr_drawer;
	}

	void internal::init_platform(const TOMLNode& node)
	{
		platform::PlatformSetup platform_setup;

		auto toml_window = node["window"];
		if (!toml_window)
			throw Error(ErrorCode::PLATFORM_INIT);
		auto width = toml_window["width"].value<int64_t>();
		auto height = toml_window["height"].value<int64_t>();
		auto title = toml_window["title"].value<std::string>();
		if (!width || !height || !title)
			throw Error(ErrorCode::PLATFORM_INIT);

		platform_setup.window_width = (int)width.value();
		platform_setup.window_height = (int)height.value();
		platform_setup.window_title = title.value();

		if (auto toml_window_hint = node["window_hint"])
		{
			reg::parse_vec(toml_window_hint["clear color"].as_array(), platform_setup.window_hint.context.clear_color);
			platform_setup.window_hint.context.swap_interval = (int)toml_window_hint["swap interval"].value<int64_t>().value_or(1);
			platform_setup.window_hint.window.resizable = toml_window_hint["resizable"].value<bool>().value_or(true);
			platform_setup.window_hint.window.visible = toml_window_hint["visible"].value<bool>().value_or(true);
			platform_setup.window_hint.window.decorated = toml_window_hint["decorated"].value<bool>().value_or(true);
			platform_setup.window_hint.window.focused = toml_window_hint["focused"].value<bool>().value_or(true);
			platform_setup.window_hint.window.auto_iconify = toml_window_hint["auto iconify"].value<bool>().value_or(true);
			platform_setup.window_hint.window.floating = toml_window_hint["floating"].value<bool>().value_or(false);
			platform_setup.window_hint.window.maximized = toml_window_hint["maximized"].value<bool>().value_or(false);
			platform_setup.window_hint.window.center_cursor = toml_window_hint["center cursor"].value<bool>().value_or(true);
			platform_setup.window_hint.window.transparent_framebuffer = toml_window_hint["transparent framebuffer"].value<bool>().value_or(false);
			platform_setup.window_hint.window.focus_on_show = toml_window_hint["focus on show"].value<bool>().value_or(true);
			platform_setup.window_hint.window.scale_to_monitor = toml_window_hint["scale to monitor"].value<bool>().value_or(false);
			platform_setup.window_hint.window.scale_framebuffer = toml_window_hint["scale framebuffer"].value<bool>().value_or(true);
			platform_setup.window_hint.window.mouse_passthrough = toml_window_hint["mouse passthrough"].value<bool>().value_or(false);
			platform_setup.window_hint.window.position_x = (unsigned int)toml_window_hint["position x"].value<int64_t>().value_or(GLFW_ANY_POSITION);
			platform_setup.window_hint.window.position_y = (unsigned int)toml_window_hint["position y"].value<int64_t>().value_or(GLFW_ANY_POSITION);
			platform_setup.window_hint.window.refresh_rate = (int)toml_window_hint["refresh rate"].value<int64_t>().value_or(GLFW_DONT_CARE);
			platform_setup.window_hint.window.stereo = toml_window_hint["stereo"].value<bool>().value_or(false);
			platform_setup.window_hint.window.srgb_capable = toml_window_hint["srgb capable"].value<bool>().value_or(false);
			platform_setup.window_hint.window.double_buffer = toml_window_hint["double buffer"].value<bool>().value_or(true);
			platform_setup.window_hint.window.opengl_forward_compat = toml_window_hint["opengl forward_compat"].value<bool>().value_or(false);
			platform_setup.window_hint.window.context_debug = toml_window_hint["context debug"].value<bool>().value_or(false);
		}

		platform_setup.num_gamepads = glm::clamp((int)node["gamepads"].value<int64_t>().value_or(0), 0, GLFW_JOYSTICK_LAST);

		internal::initial_window_size = platform_setup.window_size();
		internal::platform = platform::internal::create_platform(platform_setup);
	}

	void internal::init_viewport(const TOMLNode& node)
	{
		wr_viewport.boxed = node["window"]["boxed"].value_or<bool>(true);
		wr_viewport.stretch = node["window"]["stretch"].value_or<bool>(true);

		platform::internal::invoke_initialize_viewport(internal::wr_viewport);
		wr_viewport.attach(&internal::platform->window().handlers.window_resize);
		wr_drawer.attach(&internal::platform->window().handlers.window_resize);
	}

	void internal::terminate_platform()
	{
		internal::platform.reset();
	}

	bool internal::frame_platform()
	{
		return internal::platform->frame();
	}

	platform::Platform& get_platform()
	{
		return *internal::platform;
	}

	void set_window_resize_mode(bool boxed, bool stretch)
	{
		internal::wr_viewport.boxed = boxed;
		internal::wr_viewport.stretch = stretch;
	}

	const platform::WRViewport& get_wr_viewport()
	{
		return internal::wr_viewport;
	}

	platform::WRDrawer& get_wr_drawer()
	{
		return internal::wr_drawer;
	}

	void set_standard_viewport()
	{
		internal::wr_viewport.set_viewport();
	}

	glm::vec2 get_cursor_screen_pos()
	{
		double x, y;
		glfwGetCursorPos(internal::platform->window(), &x, &y);
		return { (float)x - 0.5f * internal::platform->window().get_width(), 0.5f * internal::platform->window().get_height() - (float)y };
	}

	glm::vec2 get_initial_window_size()
	{
		return internal::initial_window_size;
	}

	glm::vec2 get_view_stretch()
	{
		if (internal::wr_viewport.stretch)
		{
			auto v = internal::wr_viewport.get_viewport();
			return glm::vec2(v.w, v.h) / glm::vec2(internal::initial_window_size);
		}
		else
			return { 1.0f, 1.0f };
	}

	glm::vec2 get_cursor_view_pos()
	{
		return get_cursor_screen_pos() / get_view_stretch();
	}
}
