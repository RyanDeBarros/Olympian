#pragma once

#include "graphics/backend/basic/Framebuffers.h"

namespace oly::context
{
	class ScopedViewportChange
	{
		glm::vec4 original_clear_color;
		bool original_blend_enabled;

	public:
		ScopedViewportChange(glm::vec4 clear_color, bool blend_enabled, glm::ivec2 viewport_size, glm::ivec2 viewport_pos = {});
		ScopedViewportChange(const ScopedViewportChange&) = delete;
		ScopedViewportChange(ScopedViewportChange&&) = delete;
		~ScopedViewportChange();
	};

	class ScopedFullFramebufferDrawing
	{
		ScopedViewportChange viewport_change;

	public:
		ScopedFullFramebufferDrawing(const graphics::Framebuffer& framebuffer, glm::ivec2 viewport_size, glm::vec4 clear_color = glm::vec4(0.0f),
			bool blend_enabled = false, glm::ivec2 viewport_pos = {});
		ScopedFullFramebufferDrawing(const ScopedFullFramebufferDrawing&) = delete;
		ScopedFullFramebufferDrawing(ScopedFullFramebufferDrawing&&) = delete;
		~ScopedFullFramebufferDrawing();
	};
}
