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

		glm::vec2 dim;
		auto tex = other.ref.get_texture(dim);
		ref.set_texture(tex, dim);
		ref.set_tex_coords(other.ref.get_tex_coords());
		ref.set_modulation(other.ref.get_modulation());
	}

	TextGlyph::TextGlyph(TextGlyph&& other) noexcept
		: ref(std::move(other.ref)), transformer(std::move(other.transformer))
	{
		ref.set_text_glyph(true);
	}

	TextGlyph& TextGlyph::operator=(const TextGlyph& other)
	{
		if (this != &other)
		{
			ref = other.ref;
			transformer = other.transformer;

			glm::vec2 dim;
			auto tex = other.ref.get_texture(dim);
			ref.set_texture(tex, dim);
			ref.set_tex_coords(other.ref.get_tex_coords());
			ref.set_modulation(other.ref.get_modulation());
		}
		return *this;
	}

	TextGlyph& TextGlyph::operator=(TextGlyph&& other) noexcept
	{
		if (this != &other)
		{
			ref = std::move(other.ref);
			transformer = std::move(other.transformer);
			if (&ref.batch != &other.ref.batch)
			{
				glm::vec2 dim;
				auto tex = other.ref.get_texture(dim);
				ref.set_texture(tex, dim);
				ref.set_tex_coords(other.ref.get_tex_coords());
				ref.set_modulation(other.ref.get_modulation());
			}
		}
		return *this;
	}

	TextGlyph::~TextGlyph()
	{
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
		set_local() = { .position = pos - 0.5f * glm::vec2{ glyph.box.x1 + glyph.box.x2, glyph.box.y1 + glyph.box.y2 }, .scale = glyph.box.size() };
		ref.set_tex_coords(atlas.uvs(glyph));
	}
}
