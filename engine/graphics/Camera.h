#pragma once

#include "core/base/Transforms.h"
#include "core/types/SmartReference.h"

#include "core/platform/WindowEvents.h"
#include "core/platform/EventHandler.h"

namespace oly::rendering
{
	class Camera2D
	{
		struct WindowResizeHandler : public EventHandler<input::WindowResizeEventData>
		{
			glm::mat3 projection = 1.0f;

			WindowResizeHandler();

			bool block(const input::WindowResizeEventData& data) override;

			void set_projection();
		} window_resize_handler;
		friend struct WindowResizeHandler;

	public:
		Transformer2D transformer;

		glm::mat3 projection_matrix() const
		{
			return window_resize_handler.projection;
		}

		glm::mat3 view_matrix() const
		{
			return glm::inverse(transformer.global());
		}

		void attach_window_resize_handler(EventHandler<input::WindowResizeEventData>& child)
		{
			child.attach(&window_resize_handler);
		}
	};

	typedef SmartReference<Camera2D> Camera2DRef;
}
