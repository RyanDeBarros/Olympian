#pragma once

#include "core/types/Functor.h"

#include "graphics/primitives/Polygons.h"
#include "graphics/primitives/Ellipses.h"
#include "graphics/text/Paragraph.h"

namespace oly
{
	struct IRenderPipeline
	{
		virtual ~IRenderPipeline() = default;
		virtual void render_frame() const {}
	};
}

namespace oly::context
{

	extern void set_render_pipeline(const IRenderPipeline* pipeline);
	
	namespace internal
	{
		extern void render_frame();
	}

	enum class InternalBatch : int
	{
		SPRITE,
		POLYGON,
		ELLIPSE,
		TEXT,
		_COUNT
	};

	extern bool batch_is_rendering(InternalBatch batch);
	extern void flush_internal_rendering();

	namespace internal
	{
		extern void set_batch_rendering_tracker(InternalBatch batch, bool ongoing);
		extern void flush_batches_except(InternalBatch batch);
	}

	extern bool blend_enabled();
	extern glm::vec4 clear_color();
}
