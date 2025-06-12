#pragma once

#include "core/platform/Window.h"
#include "core/types/Functor.h"

namespace oly::platform
{
	struct WRViewport;
	namespace internal
	{
		extern void invoke_initialize_viewport(WRViewport& wrv);
	}

	struct WRDrawer;
	struct WRViewport : public EventHandler<input::WindowResizeEventData>
	{
		// TODO put boxed and stretch in context file
		bool boxed = true;
		bool stretch = true;

	private:
		float target_aspect_ratio = 1.0f;
		mutable glm::mat3 projection = 1.0f;
		friend struct WRDrawer;
		void set_projection() const;

		struct
		{
			float x = 0.0f, y = 0.0f, w = 0.0f, h = 0.0f;
		} viewport;

		friend void internal::invoke_initialize_viewport(WRViewport&);
		void initialize_viewport();

	public:
		bool consume(const input::WindowResizeEventData& data) override;
		void set_viewport() const;
		decltype(viewport) get_viewport() const { return viewport; }
		float get_width() const { return viewport.w; }
		float get_height() const { return viewport.h; }
		glm::vec2 get_size() const { return { viewport.w, viewport.h }; }

		const glm::mat3& get_projection() const { return projection; }
	};

	struct WRDrawer : public EventHandler<input::WindowResizeEventData>
	{
		float resizing_frame_length = 1.0f / 60.0f;

	private:
		float last_update = 0.0f;

	public:
		bool block(const input::WindowResizeEventData& data) override;
		bool consume(const input::WindowResizeEventData& data) override;
	};
}
