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

	enum class InternalBatch
	{
		NONE,
		SPRITE,
		POLYGON,
		ELLIPSE,
		TEXT
	};
	extern InternalBatch last_internal_batch_rendered();
	
	extern void invalidate_internal_batch_tracking();

	namespace internal
	{
		extern void set_last_internal_batch_rendered(InternalBatch);
	}

	extern bool blend_enabled();
	extern glm::vec4 clear_color();
}
