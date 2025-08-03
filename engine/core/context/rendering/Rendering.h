#pragma once

// TODO v3 separate Rendering.h and Registries.h into different files in subfolder corresponding to each type - one file for sprites, another for polygons, etc.

#include "core/types/Functor.h"

#include "graphics/primitives/Polygons.h"
#include "graphics/primitives/Ellipses.h"
#include "graphics/text/Paragraph.h"

namespace oly::context
{
	extern void set_render_function(const std::shared_ptr<Functor<void()>>& render_frame);

	enum class InternalBatch
	{
		NONE,
		SPRITE,
		POLYGON,
		ELLIPSE,
		TEXT
	};
	extern InternalBatch last_internal_batch_rendered();
	
	// TODO v3 set internal batch to NONE when rendering from outside engine pipeline (custom shader/rendering)

	namespace internal
	{
		extern void set_last_internal_batch_rendered(InternalBatch);
	}

	extern bool blend_enabled();
	extern glm::vec4 clear_color();
}
