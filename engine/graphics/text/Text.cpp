#include "Text.h"

#include "core/context/rendering/Sprites.h"

namespace oly::rendering
{
	TextGlyph::TextGlyph(SpriteBatch* batch)
		: ref(batch)
	{
		ref.set_text_glyph(true);
	}

	TextGlyph::TextGlyph(const TextGlyph& other)
		: ref(other.ref), transformer(other.transformer)
	{
		ref.set_text_glyph(true);
	}

	TextGlyph::TextGlyph(TextGlyph&& other) noexcept
		: ref(std::move(other.ref)), transformer(std::move(other.transformer))
	{
		ref.set_text_glyph(true);
	}

	void TextGlyph::draw() const
	{
		if (transformer.flush())
			ref.set_transform(transformer.global());
		ref.draw_quad();
		if (ref.is_in_context()) [[likely]]
			context::internal::set_sprite_batch_rendering(true);
	}

	void TextGlyph::set_glyph(const FontAtlas& atlas, const FontGlyph& glyph, glm::vec2 pos)
	{
		ref.set_texture(glyph.texture, { 1.0f, 1.0f } );
		set_local() = { .position = pos + glm::vec2{ 0.5f * glyph.box.width() + atlas.get_scale() * glyph.left_bearing, -glyph.box.center_y() - atlas.get_ascent() }, .scale = glyph.box.size()};
		ref.set_tex_coords(atlas.uvs(glyph));
	}
}
