#include "Rendering.h"

#include "core/context/rendering/Sprites.h"
#include "core/context/rendering/Polygons.h"
#include "core/context/rendering/Ellipses.h"
#include "core/context/rendering/Text.h"

namespace oly::context
{
	namespace internal
	{
		const IRenderPipeline* render_pipeline = nullptr;
		bool batch_tracking[(int)InternalBatch::_COUNT] = { false };
	}

	void set_render_pipeline(const IRenderPipeline* pipeline)
	{
		internal::render_pipeline = pipeline;
	}

	void internal::render_frame()
	{
		if (internal::render_pipeline)
			internal::render_pipeline->render_frame();
		flush_internal_rendering();
	}

	bool batch_is_rendering(InternalBatch batch)
	{
		return internal::batch_tracking[(int)batch];
	}

	void flush_internal_rendering()
	{
		if (internal::batch_tracking[(int)InternalBatch::SPRITE])
			render_sprites();
		if (internal::batch_tracking[(int)InternalBatch::POLYGON])
			render_polygons();
		if (internal::batch_tracking[(int)InternalBatch::ELLIPSE])
			render_ellipses();
		if (internal::batch_tracking[(int)InternalBatch::TEXT])
			render_text();
	}

	void internal::set_batch_rendering_tracker(InternalBatch batch, bool ongoing)
	{
		internal::batch_tracking[(int)batch] = ongoing;
	}

	void internal::flush_batches_except(InternalBatch batch)
	{
		if (batch != InternalBatch::SPRITE && internal::batch_tracking[(int)InternalBatch::SPRITE])
			render_sprites();
		if (batch != InternalBatch::POLYGON && internal::batch_tracking[(int)InternalBatch::POLYGON])
			render_polygons();
		if (batch != InternalBatch::ELLIPSE && internal::batch_tracking[(int)InternalBatch::ELLIPSE])
			render_ellipses();
		if (batch != InternalBatch::TEXT && internal::batch_tracking[(int)InternalBatch::TEXT])
			render_text();
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
