#include "WindowResize.h"

#include "core/context/Context.h"
#include "core/context/rendering/Sprites.h"
#include "core/context/Platform.h"

#include "graphics/sprites/Sprite.h"

namespace oly::platform
{
	void internal::invoke_initialize_viewport(WRViewport& wrv)
	{
		wrv.initialize_viewport();
	}

	void WRViewport::set_projection() const
	{
		glm::vec4 bounds = 0.5f * glm::vec4{ -viewport.w, viewport.w, -viewport.h, viewport.h };
		projection = glm::ortho(bounds[0], bounds[1], bounds[2], bounds[3]);
	}

	void WRViewport::initialize_viewport()
	{
		auto& window = context::get_platform().window();
		window.refresh_size();
		glm::ivec2 size = window.get_size();
		viewport.w = (float)size.x;
		viewport.h = (float)size.y;
		target_aspect_ratio = window.aspect_ratio();
		set_projection();
	}

	bool WRViewport::block(const input::WindowResizeEventData& data)
	{
		context::get_platform().window().refresh_size();
		if (boxed)
		{
			float aspect_ratio = float(data.w) / data.h;
			if (aspect_ratio > target_aspect_ratio)
			{
				viewport.h = (float)data.h;
				viewport.y = 0.0f;
				viewport.w = data.h * target_aspect_ratio;
				viewport.x = (data.w - viewport.w) * 0.5f;
			}
			else
			{
				viewport.w = (float)data.w;
				viewport.x = 0.0f;
				viewport.h = data.w / target_aspect_ratio;
				viewport.y = (data.h - viewport.h) * 0.5f;
			}
		}
		else
		{
			viewport.x = 0.0f;
			viewport.y = 0.0f;
			viewport.w = (float)data.w;
			viewport.h = (float)data.h;
		}
		if (!stretch)
			set_projection();
		return false;
	}

	void WRViewport::set_viewport() const
	{
		glViewport((int)viewport.x, (int)viewport.y, (int)viewport.w, (int)viewport.h);
	}

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
		auto& wr_viewport = context::get_wr_viewport();
		if (wr_viewport.boxed)
			glClear(GL_COLOR_BUFFER_BIT); // TODO v6 clear depth buffer too?
		wr_viewport.set_viewport();
		return !context::frame();
	}
}
