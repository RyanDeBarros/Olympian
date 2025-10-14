#pragma once

#include "core/base/Transforms.h"
#include "core/types/SmartReference.h"
#include "core/math/Shapes.h"

#include "core/platform/WindowEvents.h"
#include "core/platform/EventHandler.h"

namespace oly::rendering
{
	class Camera2D;

	namespace internal
	{
		extern void initialize(Camera2D&, bool boxed, bool stretch);
	}

	class Camera2D : public EventHandler<input::WindowResizeEventData>
	{
		bool boxed = true;
		bool stretch = true;

		float target_aspect_ratio = 1.0f;
		math::Area2D viewport;

		glm::mat3 projection = 1.0f;
	
	public:
		Transformer2D transformer;

		Camera2D();

		bool block(const input::WindowResizeEventData& data) override;
		bool consume(const input::WindowResizeEventData& data) override;

	private:
		friend void internal::initialize(Camera2D&, bool boxed, bool stretch);
		void initialize(bool boxed, bool stretch);
		void set_projection();

	public:
		glm::mat3 projection_matrix() const;

		math::Area2D get_viewport() const { return viewport; }
		void apply_viewport() const;

		bool is_boxed() const { return boxed; }
		bool is_stretch() const { return stretch; }
	};

	typedef SmartReference<Camera2D> Camera2DRef;
}
