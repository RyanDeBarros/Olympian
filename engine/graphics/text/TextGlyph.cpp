#include "TextGlyph.h"

#include "core/context/rendering/Sprites.h"

namespace oly::rendering
{
    TextGlyph::TextGlyph(Unbatched)
        : ref_(kUnbatched)
    {
    }

    TextGlyph::TextGlyph(SpriteBatch& batch)
        : ref_(batch)
    {
        ref_.set_text_glyph(true);
    }

    TextGlyph::TextGlyph(const TextGlyph& other)
        : ref_(other.ref_)
        , transformer(other.transformer)
    {
        ref_.set_text_glyph(true);
    }

    TextGlyph::TextGlyph(TextGlyph&& other) noexcept
        : ref_(std::move(other.ref_))
        , transformer(std::move(other.transformer))
    {
        ref_.set_text_glyph(true);
    }

    internal::SpriteReference::accessor TextGlyph::get_batch() const
    {
        return ref_.get_batch();
    }

    void TextGlyph::set_batch(Unbatched)
    {
        ref_.set_batch(kUnbatched);
    }

    void TextGlyph::set_batch(SpriteBatch& batch)
    {
        ref_.set_batch(batch);
        ref_.set_text_glyph(true);
    }

    void TextGlyph::set_camera_invariant(bool is_camera_invariant) const
    {
        ref_.set_camera_invariant(is_camera_invariant);
    }

    bool TextGlyph::is_camera_invariant() const
    {
        return ref_.is_camera_invariant();
    }

    void TextGlyph::draw() const
    {
        if (transformer.flush())
            ref_.set_transform(transformer.global());

        ref_.draw_quad();
    }

    void TextGlyph::set_text_color(glm::vec4 color) const
    {
        ref_.set_modulation(color);
    }

    glm::vec4 TextGlyph::get_text_color() const
    {
        return ref_.get_modulation();
    }

    const Transform2D& TextGlyph::get_local() const
    {
        return transformer.get_local();
    }
    
    Transform2D& TextGlyph::set_local()
    {
        return transformer.set_local();
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
        ref_.set_texture(texture, { 1.0f, 1.0f });
        math::Rect2D box = unscaled_box.get_scaled(scale);
        set_local() = {
            .position = pos + glm::vec2{ 0.5f * box.width() + left_bearing, -box.center_y() - ascent },
            .scale = box.size()
        };
        ref_.set_tex_coords(uvs);
    }
}
