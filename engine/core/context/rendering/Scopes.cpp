#include "Scopes.h"

#include "core/context/rendering/Rendering.h"
#include "core/context/Platform.h"

#include "core/base/Assert.h"

namespace oly::context
{
	ScopedViewportChange::ScopedViewportChange(const rendering::Camera2D& camera, glm::vec4 clear_color, bool blend_enabled, glm::ivec2 viewport_size, glm::ivec2 viewport_pos)
		: camera(camera)
	{
		original_clear_color = context::clear_color();
		original_blend_enabled = context::blend_enabled();
		glViewport(viewport_pos.x, viewport_pos.y, viewport_size.x, viewport_size.y);
		glClearColor(clear_color[0], clear_color[1], clear_color[2], clear_color[3]);
		if (blend_enabled)
			glEnable(GL_BLEND);
		else
			glDisable(GL_BLEND);
	}

	ScopedViewportChange::~ScopedViewportChange()
	{
		camera.apply_viewport();
		glClearColor(original_clear_color[0], original_clear_color[1], original_clear_color[2], original_clear_color[3]);
		if (original_blend_enabled)
			glEnable(GL_BLEND);
		else
			glDisable(GL_BLEND);
	}
}
