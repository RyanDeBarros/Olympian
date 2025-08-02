#pragma once

// TODO v3 separate Rendering.h and Registries.h into different files in subfolder corresponding to each type - one file for sprites, another for polygons, etc.

#include "core/types/Functor.h"

#include "graphics/primitives/Sprites.h"
#include "graphics/primitives/Polygons.h"
#include "graphics/primitives/Ellipses.h"
#include "graphics/text/Paragraph.h"

namespace oly::graphics
{
	class NSVGContext;
}

namespace oly::context
{
	extern void set_render_function(const std::shared_ptr<Functor<void()>>& render_frame);

	extern rendering::SpriteBatch& sprite_batch();
	extern rendering::PolygonBatch& polygon_batch();
	extern rendering::EllipseBatch& ellipse_batch();
	extern rendering::TextBatch& text_batch();

	enum class InternalBatch
	{
		NONE,
		SPRITE,
		POLYGON,
		ELLIPSE,
		TEXT
	};
	extern InternalBatch last_internal_batch_rendered();

	extern graphics::NSVGContext& nsvg_context();

	extern void sync_texture_handle(const graphics::BindlessTextureRef& texture);

	extern void render_sprites();
	extern void render_polygons();
	extern void render_ellipses();
	extern void render_text();

	extern bool blend_enabled();
	extern glm::vec4 clear_color();
}
