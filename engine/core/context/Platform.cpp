#include "Platform.h"

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
		platform::PlatformSetup platform_setup(node);
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
