#include "Rendering.h"

#include "core/context/rendering/Sprites.h"

namespace oly::context
{
	namespace internal
	{
		const IRenderPipeline* render_pipeline = nullptr;
	}

	void set_render_pipeline(const IRenderPipeline* pipeline)
	{
		internal::render_pipeline = pipeline;
	}

	void internal::render_frame()
	{
		if (internal::render_pipeline)
			internal::render_pipeline->render_frame();
		render_sprites();
	}

	bool blend_enabled()
	{
		GLboolean enabled;
		glGetBooleanv(GL_BLEND, &enabled);
		return (bool)enabled;
	}

	glm::vec4 clear_color()
	{
		glm::vec4 color;
		glGetFloatv(GL_COLOR_CLEAR_VALUE, glm::value_ptr(color));
		return color;
	}
}
