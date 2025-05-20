#pragma once

#include "core/platform/Window.h"
#include "core/types/Functor.h"

namespace oly::platform
{
	struct StandardWindowResize : public EventHandler<input::WindowResizeEventData>
	{
		bool boxed = true;
		bool stretch = true;

		float target_aspect_ratio;
		glm::vec4 projection_bounds;

		std::shared_ptr<Functor<void()>> render_frame;

		bool consume(const input::WindowResizeEventData& data);

	private:
		bool re_render_frame() const;
		void set_projection_bounds(glm::vec4 projection_bounds);
	};
}
