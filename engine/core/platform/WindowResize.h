#pragma once

#include "core/platform/Window.h"
#include "core/types/Functor.h"

namespace oly::platform
{
	struct StandardWindowResize : public EventHandler<input::WindowResizeEventData>
	{
		// TODO put boxed, stretch, and target_aspect_ratio in configuration file
		bool boxed = true;
		bool stretch = true;

		float target_aspect_ratio;
		glm::vec4 projection_bounds;

		std::shared_ptr<Functor<void()>> render_frame;

	private:
		struct
		{
			float x = 0.0f, y = 0.0f, w = 0.0f, h = 0.0f;
		} viewport;

	public:
		void initialize_viewport();
		bool consume(const input::WindowResizeEventData& data) override;
		void set_viewport() const;
		decltype(viewport) get_viewport() const { return viewport; }

	private:
		bool re_render_frame() const;
		void set_projection_bounds(glm::vec4 projection_bounds);
	};

	// TODO split StandardWindowResize into two - WRViewport and WRDrawer
}
