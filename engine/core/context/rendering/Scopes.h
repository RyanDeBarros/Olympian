#pragma once

#include "graphics/backend/basic/Framebuffers.h"
#include "graphics/Camera.h"

namespace oly::context
{
	class ScopedViewportChange
	{
		const rendering::Camera2D& camera;
		glm::vec4 original_clear_color;
		bool original_blend_enabled;

	public:
		ScopedViewportChange(const rendering::Camera2D& camera, glm::vec4 clear_color, bool blend_enabled, glm::ivec2 viewport_size, glm::ivec2 viewport_pos = {});
		ScopedViewportChange(const ScopedViewportChange&) = delete;
		ScopedViewportChange(ScopedViewportChange&&) = delete;
		~ScopedViewportChange();
	};
}
