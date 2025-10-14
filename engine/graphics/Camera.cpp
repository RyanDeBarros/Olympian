#include "Camera.h"

#include "core/context/Platform.h"

namespace oly::rendering
{
	Camera2D::Camera2D()
	{
		auto& window = context::get_platform().window();
		window.refresh_size();
		glm::ivec2 size = window.get_size();
		viewport.w = (float)size.x;
		viewport.h = (float)size.y;
		target_aspect_ratio = window.aspect_ratio();

		set_projection();
		attach(&window.handlers.window_resize);
	}

	bool Camera2D::block(const input::WindowResizeEventData& data)
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

	bool Camera2D::consume(const input::WindowResizeEventData&)
	{
		if (boxed)
			glClear(GL_COLOR_BUFFER_BIT); // TODO v6 clear depth buffer too?
		apply_viewport();
		return false;
	}

	void Camera2D::set_projection()
	{
		glm::vec4 bounds = 0.5f * glm::vec4{ -viewport.w, viewport.w, -viewport.h, viewport.h };
		projection = glm::ortho(bounds[0], bounds[1], bounds[2], bounds[3]);
	}

	glm::mat3 Camera2D::projection_matrix() const
	{
		// TODO v6 for ui widgets that are in camera-space, only return projection - define two different projection uniforms in shader - one for world space and one for camera space.
		return projection * glm::inverse(transformer.global());
	}

	void Camera2D::apply_viewport() const
	{
		glViewport((int)viewport.x, (int)viewport.y, (int)viewport.w, (int)viewport.h);
	}
}
