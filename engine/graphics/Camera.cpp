#include "Camera.h"

#include "core/context/Platform.h"

namespace oly::rendering
{
	void internal::initialize_default_camera(bool boxed, bool stretch)
	{
		Camera2DRef(REF_DEFAULT)->reinitialize(boxed, stretch);
	}

	Camera2D::Camera2D(bool boxed, bool stretch)
	{
		attach(&context::get_platform().window().handlers.window_resize);
		reinitialize(boxed, stretch);
	}

	bool Camera2D::block(const input::WindowResizeEventData& data)
	{
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
			glClear(GL_COLOR_BUFFER_BIT); // TODO v8 clear depth buffer too?
		apply_viewport();
		return false;
	}

	void Camera2D::reinitialize(bool boxed, bool stretch)
	{
		this->boxed = boxed;
		this->stretch = stretch;

		glm::ivec2 size = context::get_platform().window().get_size();
		viewport.w = (float)size.x;
		viewport.h = (float)size.y;
		target_aspect_ratio = viewport.w / viewport.h;
		set_projection();
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

	// TODO v5 add uInvariantProjection to ellipse and polygon shaders.
	glm::mat3 Camera2D::invariant_projection_matrix() const
	{
		return projection;
	}

	void Camera2D::apply_viewport() const
	{
		glViewport((int)viewport.x, (int)viewport.y, (int)viewport.w, (int)viewport.h);
	}
	
	glm::vec2 Camera2D::screen_to_world_coordinates(glm::vec2 screen_pos) const
	{
		glm::vec2 ndc = {
			((screen_pos.x - viewport.x) / viewport.w) * 2.0f - 1.0f,
			((screen_pos.y - viewport.y) / viewport.h) * 2.0f - 1.0f
		};
		return glm::vec2(glm::inverse(projection_matrix()) * glm::vec3{ ndc.x, -ndc.y, 1.0f });
	}
	
	glm::vec2 Camera2D::world_to_screen_coordinates(glm::vec2 world_pos) const
	{
		glm::vec2 ndc = projection_matrix() * glm::vec3(world_pos, 1.0f);
		return {
			(ndc.x + 1.0f) * 0.5f * viewport.w + viewport.x,
			(-ndc.y + 1.0f) * 0.5f * viewport.h + viewport.y,
		};
	}
	
	glm::vec2 Camera2D::view_to_world_coordinates(glm::vec2 view_pos) const
	{
		return screen_to_world_coordinates(context::get_platform().window().view_to_screen_coordinates(view_pos));
	}
	
	glm::vec2 Camera2D::world_to_view_coordinates(glm::vec2 world_pos) const
	{
		return context::get_platform().window().screen_to_view_coordinates(world_to_screen_coordinates(world_pos));
	}
	
	glm::vec2 Camera2D::get_cursor_world_position() const
	{
		return screen_to_world_coordinates(context::get_platform().window().get_cursor_screen_position());
	}
}
