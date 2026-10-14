#pragma once

#include "core/base/SmartReference.h"
#include "core/base/Transforms.h"

#include "core/math/Shapes.h"

#include "core/platform/EventHandler.h"
#include "core/platform/WindowEvents.h"

namespace oly::rendering
{
	namespace internal
	{
		extern void initialize_default_camera(bool boxed, bool stretch);
	}

	class Camera2D : public EventHandler<input::WindowResizeEventData>
	{
        friend void internal::initialize_default_camera(bool boxed, bool stretch);

    private:
		bool boxed_ = true;
		bool stretch_ = true;

		float target_aspect_ratio_ = 1.0f;
		math::Area2D viewport_;

		glm::mat3 projection_ = 1.0f;
	
	public:
		Transformer2D transformer;

		Camera2D(bool boxed = true, bool stretch = true);

	public:
		bool block(const input::WindowResizeEventData& data) override;
		bool consume(const input::WindowResizeEventData& data) override;

		glm::mat3 projection_matrix() const;
		glm::mat3 invariant_projection_matrix() const;

		void project_to_rect(math::Rect2D rect);

        math::Area2D get_viewport() const;
		void apply_viewport() const;

		bool is_boxed() const;
		bool is_stretch() const;

		glm::vec2 screen_to_world_coordinates(glm::vec2 screen_pos) const;
		glm::vec2 world_to_screen_coordinates(glm::vec2 world_pos) const;
		glm::vec2 view_to_world_coordinates(glm::vec2 view_pos) const;
		glm::vec2 world_to_view_coordinates(glm::vec2 world_pos) const;
		glm::vec2 get_cursor_world_position() const;

    private:
        void reinitialize(bool boxed, bool stretch);
        void set_projection();
	};

	using Camera2DRef = SmartReference<Camera2D>;
}
