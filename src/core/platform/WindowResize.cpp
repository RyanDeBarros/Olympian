#include "WindowResize.h"

#include "core/base/Context.h"

namespace oly::platform
{
	bool StandardWindowResize::consume(const input::WindowResizeEventData& data)
	{
		context::get_platform().window().refresh_size();
		int vx, vy, vw, vh;
		if (boxed)
		{
			float aspect_ratio = float(data.w) / data.h;
			if (aspect_ratio > target_aspect_ratio)
			{
				vh = data.h;
				vy = 0;
				vw = (int)(data.h * target_aspect_ratio);
				vx = (data.w - vw) / 2;
			}
			else
			{
				vw = data.w;
				vx = 0;
				vh = (int)(data.w / target_aspect_ratio);
				vy = (data.h - vh) / 2;
			}
			glClear(GL_COLOR_BUFFER_BIT); // LATER clear depth buffer too?
		}
		else
		{
			vx = 0;
			vy = 0;
			vw = data.w;
			vh = data.h;
		}
		glViewport(vx, vy, vw, vh);
		if (!stretch)
			set_projection_bounds(0.5f * glm::vec4{ -vw, vw, -vh, vh });
		return !re_render_frame();
	}

	bool StandardWindowResize::re_render_frame() const
	{
		if (render_frame)
			render_frame();
		return context::frame();
	}

	void StandardWindowResize::set_projection_bounds(glm::vec4 projection_bounds) const
	{
		context::sprite_batch().projection_bounds = projection_bounds;
		context::polygon_batch().projection_bounds = projection_bounds;
		context::ellipse_batch().projection_bounds = projection_bounds;
		context::text_batch().projection_bounds = projection_bounds;
	}
}
