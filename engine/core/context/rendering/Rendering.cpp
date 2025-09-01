#include "Rendering.h"

namespace oly::context
{
	namespace internal
	{
		InternalBatch last_internal_batch_rendered = InternalBatch::NONE;
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
