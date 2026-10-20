#pragma once

#include "core/base/Color.h"
#include "core/base/ControlValues.h"

#include "graphics/text/FontFamily.h"

#include <imp/utf.hpp>

namespace oly::rendering
{
    class TextGlyph;

    struct Font
    {
        using Variant = imp::variant<FontSelection, FontAtlasRef, RasterFontRef>;

        Variant f;

        Font() = default;
        Font(const FontAtlasRef& font);
        Font(const RasterFontRef& font);
        Font(const FontSelection& font);
        Font& operator=(const FontAtlasRef& font);
        Font& operator=(const RasterFontRef& font);
        Font& operator=(const FontSelection& font);

        void apply_style(FontStyle style);
        void unapply_style(FontStyle style);
        void reset_style();

        float line_height() const;

        bool operator==(const FontAtlasRef& font) const;
        bool operator==(const RasterFontRef& font) const;
        bool operator==(const FontSelection& font) const;
        bool operator==(const Font& other) const;

        bool adj_compat(const Font& other) const;
        bool support(imp::utf::codepoint c) const;
        void set_glyph(TextGlyph& glyph, imp::utf::codepoint c, glm::vec2 pos, glm::vec2 scale) const;
        float advance_width(imp::utf::codepoint c, imp::utf::codepoint next_codepoint) const;
    };

    struct TextElement
    {
        Font font;
        imp::utf::string text = "";
        Color text_color = colors::kWhite;
        float adj_offset = 0.f;
        glm::vec2 scale = glm::vec2(1.f);
        BUInterval line_y_pivot = 0.f;
        glm::vec2 jitter_offset = {};

        static std::vector<TextElement> expand(const TextElement& element);
        static void expand(const TextElement& element, std::vector<TextElement>& to);

        float line_height() const;

        void set_glyph(TextGlyph& glyph, imp::utf::codepoint c, glm::vec2 pos) const;
        float advance_width(imp::utf::codepoint c, imp::utf::codepoint next_codepoint) const;
    };
}
