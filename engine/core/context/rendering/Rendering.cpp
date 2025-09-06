#include "Rendering.h"

namespace oly::context
{
	namespace internal
	{
		const IRenderPipeline* render_pipeline = nullptr;
		InternalBatch last_internal_batch_rendered = InternalBatch::NONE;
	}

	void set_render_pipeline(const IRenderPipeline* pipeline)
	{
		internal::render_pipeline = pipeline;
	}

	void internal::render_frame()
	{
		if (internal::render_pipeline)
			internal::render_pipeline->render_frame();
	}

	InternalBatch last_internal_batch_rendered()
	{
		return internal::last_internal_batch_rendered;
	}

	void invalidate_internal_batch_tracking()
	{
		internal::set_last_internal_batch_rendered(InternalBatch::NONE);
	}

	void internal::set_last_internal_batch_rendered(InternalBatch batch)
	{
		internal::last_internal_batch_rendered = batch;
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
