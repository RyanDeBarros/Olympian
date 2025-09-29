#pragma once

#include "graphics/sprites/SpriteBatch.h"
#include "graphics/text/Font.h"
#include "core/base/Transforms.h"

namespace oly::rendering
{
	class TextGlyph
	{
		internal::SpriteReference ref;

	public:
		Transformer2D transformer;

		TextGlyph() = default;
		TextGlyph(Unbatched);
		TextGlyph(SpriteBatch& batch);
		TextGlyph(const TextGlyph&);
		TextGlyph(TextGlyph&&) noexcept;

		auto get_batch() const { return ref.get_batch(); }
		void set_batch(Unbatched) { ref.set_batch(UNBATCHED); }
		void set_batch(SpriteBatch& batch);

		void draw() const;

		void set_glyph(const FontAtlas& atlas, const FontGlyph& glyph, glm::vec2 pos, glm::vec2 scale);
		void set_text_color(glm::vec4 color) const { ref.set_modulation(color); }
		glm::vec4 get_text_color() const { return ref.get_modulation(); }

		const Transform2D& get_local() const { return transformer.get_local(); }
		Transform2D& set_local() { return transformer.set_local(); }
	};
}
