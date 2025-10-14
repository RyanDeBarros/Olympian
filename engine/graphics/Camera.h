#pragma once

#include "core/base/Transforms.h"
#include "core/types/SmartReference.h"
#include "core/math/Shapes.h"

#include "core/platform/WindowEvents.h"
#include "core/platform/EventHandler.h"

namespace oly::rendering
{
	class Camera2D : public EventHandler<input::WindowResizeEventData>
	{
	public:
		bool boxed = true;
		bool stretch = true;

	private:
		float target_aspect_ratio = 1.0f;
		math::Area2D viewport;

		glm::mat3 projection = 1.0f;
	
	public:
		Transformer2D transformer;

		Camera2D();

		bool block(const input::WindowResizeEventData& data) override;
		bool consume(const input::WindowResizeEventData& data) override;

	private:
		void set_projection();

	public:
		glm::mat3 projection_matrix() const;

		math::Area2D get_viewport() const { return viewport; }
		void apply_viewport() const;
	};

	typedef SmartReference<Camera2D> Camera2DRef;
}
