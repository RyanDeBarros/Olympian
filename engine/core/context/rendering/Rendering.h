#pragma once

#include "external/GLM.h"

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

	extern bool blend_enabled();
	extern glm::vec4 clear_color();
}
