#include "TextGlyph.h"

#include "core/context/rendering/Sprites.h"

namespace oly::rendering
{
	TextGlyph::TextGlyph(Unbatched)
		: ref(UNBATCHED)
	{
	}

	TextGlyph::TextGlyph(SpriteBatch& batch)
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

	void TextGlyph::set_batch(SpriteBatch& batch)
	{
		ref.set_batch(batch);
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

	void TextGlyph::set_glyph(const FontAtlas& atlas, const FontGlyph& glyph, glm::vec2 pos, glm::vec2 scale)
	{
		set_glyph(glyph.texture(), (math::Rect2D)glyph.box(), pos, scale, atlas.get_scale() * glyph.left_bearing(), atlas.get_ascent(), atlas.uvs(glyph));
	}

	void TextGlyph::set_glyph(const RasterFont& font, const RasterFontGlyph& glyph, glm::vec2 pos, glm::vec2 scale)
	{
		set_glyph(glyph.texture(), glyph.box().get_scaled(font.get_scale()), pos, scale, font.get_scale().x * glyph.left_bearing(), font.line_height(), glyph.uvs());
	}

	void TextGlyph::set_glyph(const graphics::BindlessTextureRef& texture, math::Rect2D unscaled_box, glm::vec2 pos, glm::vec2 scale,
		float left_bearing, float ascent, math::UVRect uvs)
	{
		ref.set_texture(texture, { 1.0f, 1.0f });
		math::Rect2D box = unscaled_box.get_scaled(scale);
		set_local() = {
			.position = pos + glm::vec2{ 0.5f * box.width() + left_bearing, -box.center_y() - ascent },
			.scale = box.size()
		};
		ref.set_tex_coords(uvs);
	}
}
