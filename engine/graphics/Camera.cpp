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
		if (boxed_)
		{
			float aspect_ratio = float(data.w) / data.h;
			if (aspect_ratio > target_aspect_ratio_)
			{
				viewport_.h = (float)data.h;
				viewport_.y = 0.0f;
				viewport_.w = data.h * target_aspect_ratio_;
				viewport_.x = (data.w - viewport_.w) * 0.5f;
			}
			else
			{
				viewport_.w = (float)data.w;
				viewport_.x = 0.0f;
				viewport_.h = data.w / target_aspect_ratio_;
				viewport_.y = (data.h - viewport_.h) * 0.5f;
			}
		}
		else
		{
			viewport_.x = 0.0f;
			viewport_.y = 0.0f;
			viewport_.w = (float)data.w;
			viewport_.h = (float)data.h;
		}

		if (!stretch_)
			set_projection();

		return false;
	}

	bool Camera2D::consume(const input::WindowResizeEventData&)
	{
		if (boxed_)
		{
			glClear(GL_COLOR_BUFFER_BIT);
			glClear(GL_DEPTH_BUFFER_BIT);
		}

		apply_viewport();
		return false;
	}

	glm::mat3 Camera2D::projection_matrix() const
	{
		return projection_ * glm::inverse(transformer.global());
	}

	glm::mat3 Camera2D::invariant_projection_matrix() const
	{
		return projection_;
	}

	void Camera2D::project_to_rect(math::Rect2D rect)
	{
		viewport_.x = rect.x1;
		viewport_.y = rect.y1;
		viewport_.w = rect.width();
		viewport_.h = rect.height();

		set_projection();
		transformer.set_local().position = rect.center();
	}

    math::Area2D Camera2D::get_viewport() const
    {
        return viewport_;
    }

	void Camera2D::apply_viewport() const
	{
		glViewport((int)viewport_.x, (int)viewport_.y, (int)viewport_.w, (int)viewport_.h);
	}

    bool Camera2D::is_boxed() const
    {
        return boxed_;
    }

    bool Camera2D::is_stretch() const
    {
        return stretch_;
    }

	glm::vec2 Camera2D::screen_to_world_coordinates(glm::vec2 screen_pos) const
	{
		glm::vec2 ndc = {
			((screen_pos.x - viewport_.x) / viewport_.w) * 2.0f - 1.0f,
			((screen_pos.y - viewport_.y) / viewport_.h) * 2.0f - 1.0f
		};
		return glm::vec2(glm::inverse(projection_matrix()) * glm::vec3{ ndc.x, -ndc.y, 1.0f });
	}
	
	glm::vec2 Camera2D::world_to_screen_coordinates(glm::vec2 world_pos) const
	{
		glm::vec2 ndc = projection_matrix() * glm::vec3(world_pos, 1.0f);
		return {
			(ndc.x + 1.0f) * 0.5f * viewport_.w + viewport_.x,
			(-ndc.y + 1.0f) * 0.5f * viewport_.h + viewport_.y,
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

    void Camera2D::reinitialize(bool boxed, bool stretch)
    {
        this->boxed_ = boxed;
        this->stretch_ = stretch;

        glm::ivec2 size = context::get_platform().window().get_size();
        viewport_.w = (float)size.x;
        viewport_.h = (float)size.y;
        target_aspect_ratio_ = viewport_.w / viewport_.h;
        set_projection();
    }

    void Camera2D::set_projection()
    {
        projection_ = glm::ortho(-0.5f * viewport_.w, 0.5f * viewport_.w, -0.5f * viewport_.h, 0.5f * viewport_.h);
    }
}
