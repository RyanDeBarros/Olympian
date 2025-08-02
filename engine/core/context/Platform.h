#pragma once

// TODO v3 separate into Platform.h and Collision.h

#include "core/platform/Platform.h"
#include "core/platform/WindowResize.h"

namespace oly
{
	namespace col2d
	{
		class CollisionDispatcher;
	}
}

namespace oly::context
{
	extern platform::Platform& get_platform();
	extern void set_window_resize_mode(bool boxed = true, bool stretch = true);
	extern const platform::WRViewport& get_wr_viewport();
	extern platform::WRDrawer& get_wr_drawer();
	extern void set_standard_viewport();

	extern glm::vec2 get_cursor_screen_pos();
	extern glm::vec2 get_initial_window_size();
	extern glm::vec2 get_view_stretch();
	extern glm::vec2 get_cursor_view_pos();

	extern col2d::CollisionDispatcher& collision_dispatcher();
}
