#include "WindowResize.h"

#include "core/base/Context.h"

namespace oly::platform
{
	void StandardWindowResize::initialize_viewport()
	{
		auto& window = context::get_platform().window();
		window.refresh_size();
		glm::ivec2 size = window.get_size();
		viewport.w = size.x;
		viewport.h = size.y;
		projection_bounds = 0.5f * glm::vec4{ -size.x, size.x, -size.y, size.y };
	}

	bool StandardWindowResize::consume(const input::WindowResizeEventData& data)
	{
		context::get_platform().window().refresh_size();
		if (boxed)
		{
			float aspect_ratio = float(data.w) / data.h;
			if (aspect_ratio > target_aspect_ratio)
			{
				viewport.h = data.h;
				viewport.y = 0;
				viewport.w = data.h * target_aspect_ratio;
				viewport.x = (data.w - viewport.w) * 0.5f;
			}
			else
			{
				viewport.w = data.w;
				viewport.x = 0;
				viewport.h = data.w / target_aspect_ratio;
				viewport.y = (data.h - viewport.h) * 0.5f;
			}
			glClear(GL_COLOR_BUFFER_BIT); // LATER clear depth buffer too?
		}
		else
		{
			viewport.x = 0;
			viewport.y = 0;
			viewport.w = data.w;
			viewport.h = data.h;
		}
		set_viewport();
		if (!stretch)
			set_projection_bounds(0.5f * glm::vec4{ -viewport.w, viewport.w, -viewport.h, viewport.h });
		return !re_render_frame();
	}

	void StandardWindowResize::set_viewport() const
	{
		glViewport((int)viewport.x, (int)viewport.y, (int)viewport.w, (int)viewport.h);
	}

	bool StandardWindowResize::re_render_frame() const
	{
		if (render_frame)
			(*render_frame)();
		return context::frame();
	}

	void StandardWindowResize::set_projection_bounds(glm::vec4 projection_bounds)
	{
		this->projection_bounds = projection_bounds;
		context::sprite_batch().projection_bounds = projection_bounds;
		context::polygon_batch().projection_bounds = projection_bounds;
		context::ellipse_batch().projection_bounds = projection_bounds;
		context::text_batch().projection_bounds = projection_bounds;
	}
}
