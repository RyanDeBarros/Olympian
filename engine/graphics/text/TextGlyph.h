#pragma once

#include "core/base/Transforms.h"

#include "graphics/sprites/SpriteBatch.h"

#include "graphics/text/Font.h"
#include "graphics/text/RasterFont.h"

namespace oly::rendering
{
    class TextGlyph
    {
        internal::SpriteReference ref_;

    public:
        Transformer2D transformer;

        TextGlyph() = default;
        TextGlyph(Unbatched);
        TextGlyph(SpriteBatch& batch);
        TextGlyph(const TextGlyph&);
        TextGlyph(TextGlyph&&) noexcept;

    public:
        internal::SpriteReference::accessor get_batch() const;
        void set_batch(Unbatched);
        void set_batch(SpriteBatch& batch);

        void set_camera_invariant(bool is_camera_invariant) const;
        bool is_camera_invariant() const;

        void draw() const;

        void set_text_color(glm::vec4 color) const;
        glm::vec4 get_text_color() const;

        const Transform2D& get_local() const;
        Transform2D& set_local();

        void set_glyph(const FontAtlas& atlas, const FontGlyph& glyph, glm::vec2 pos, glm::vec2 scale);
        void set_glyph(const RasterFont& font, const RasterFontGlyph& glyph, glm::vec2 pos, glm::vec2 scale);

    private:
        void set_glyph(
            const graphics::BindlessTextureRef& texture,
            math::Rect2D unscaled_box,
            glm::vec2 pos,
            glm::vec2 scale,
            float left_bearing,
            float ascent,
            math::UVRect uvs);
    };
}
