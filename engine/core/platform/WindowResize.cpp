#include "WindowResize.h"

#include "core/context/Context.h"
#include "core/context/rendering/Sprites.h"
#include "core/context/Platform.h"

#include "graphics/sprites/Sprite.h"

namespace oly::platform
{
	bool WRDrawer::block(const input::WindowResizeEventData& data)
	{
		float now = (float)glfwGetTime();
		float elapsed = now - last_update;
		if (elapsed < resizing_frame_length)
			return true;
		else
		{
			last_update = now;
			return false;
		}
	}

	bool WRDrawer::consume(const input::WindowResizeEventData& data)
	{
		return !context::frame(); // TODO v5 FIX: looks weird if not doing logic update
	}
}
