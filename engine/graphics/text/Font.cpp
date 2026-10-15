#include "Font.h"

#include "core/base/Errors.h"

#include "core/util/IO.h"
#include "core/util/Logger.h"
#include "core/util/Memory.h"

namespace oly
{
    namespace rendering
    {
        FontFace::FontFace(const detail::ResourcePath& font_file, Kerning&& kerning)
            : info_{}
            , data_(io::read_file_uc(font_file))
            , kerning_(std::move(kerning))
        {
            if (!stbtt_InitFont(&info_, data_.data(), 0))
            {
                _OLY_ENGINE_LOG_ERROR("RENDERING") << "Cannot initialize font_" << LOG.endl;
                throw Error(ErrorCode::LoadFont);
            }
        }

        float FontFace::scale_for_pixel_height(float font_size) const
        {
            return stbtt_ScaleForPixelHeight(&info_, font_size);
        }

        void FontFace::get_glyph_horizontal_metrics(int glyph_index, int& advance_width, int& left_bearing) const
        {
            stbtt_GetGlyphHMetrics(&info_, glyph_index, &advance_width, &left_bearing);
        }

        void FontFace::get_codepoint_horizontal_metrics(imp::utf::codepoint codepoint, int& advance_width, int& left_bearing) const
        {
            stbtt_GetCodepointHMetrics(&info_, codepoint, &advance_width, &left_bearing);
        }

        void FontFace::get_vertical_metrics(int& ascent_, int& descent, int& linegap) const
        {
            stbtt_GetFontVMetrics(&info_, &ascent_, &descent, &linegap);
        }

        int FontFace::find_glyph_index(imp::utf::codepoint codepoint) const
        {
            return stbtt_FindGlyphIndex(&info_, codepoint);
        }

        void FontFace::get_bitmap_box(int glyph_index, float scale_, int& ch_x0, int& ch_x1, int& ch_y0, int& ch_y1) const
        {
            return stbtt_GetGlyphBitmapBox(&info_, glyph_index, scale_, scale_, &ch_x0, &ch_y0, &ch_x1, &ch_y1);
        }

        void FontFace::make_bitmap(unsigned char* buf, int w, int h, float scale_, int glyph_index) const
        {
            stbtt_MakeGlyphBitmap(&info_, buf, w, h, w, scale_, scale_, glyph_index);
            flip_pixel_buffer(buf, w, h, 1);
        }

        int FontFace::get_kerning(imp::utf::codepoint c1, imp::utf::codepoint c2) const
        {
            auto k = kerning_.map.find({ c1, c2 });
            if (k != kerning_.map.end()) [[unlikely]]
                return k->second;
            else
            {
                int g1 = find_glyph_index(c1);
                int g2 = find_glyph_index(c2);
                return stbtt_GetGlyphKernAdvance(&info_, g1, g2);
            }
        }

        FontGlyph::FontGlyph(const FontAtlas& font_, int index, float scale_, size_t buffer_pos)
            : index_(index)
            , buffer_pos_(buffer_pos)
        {
            font_.font_->get_glyph_horizontal_metrics(index, advance_width_, left_bearing_);
            font_.font_->get_bitmap_box(index, scale_, box_.x1, box_.x2, box_.y1, box_.y2);
        }

        math::IRect2D FontGlyph::box() const
        {
            return box_;
        }

        int FontGlyph::advance_width() const
        {
            return advance_width_;
        }

        int FontGlyph::left_bearing() const
        {
            return left_bearing_;
        }

        graphics::BindlessTextureRef FontGlyph::texture() const
        {
            return texture_;
        }

        void FontGlyph::render_on_bitmap_shared(const FontAtlas& font_, unsigned char* buffer, int w, int h, int left_padding, int right_padding, int bottom_padding, int top_padding) const
        {
            int width = box_.width();
            int height = box_.height();
            unsigned char* temp = new unsigned char[width * height];

            font_.font_->make_bitmap(temp, width, height, font_.scale_, index_);

            for (int row = 0; row < bottom_padding; ++row)
                memset(buffer + row * w, 0, left_padding + width + right_padding);

            for (int row = bottom_padding; row < bottom_padding + height; ++row)
            {
                memset(buffer + row * w, 0, left_padding);
                memcpy(buffer + row * w + left_padding, temp + (row - bottom_padding) * width, width);
                memset(buffer + row * w + left_padding + width, 0, right_padding);
            }

            for (int row = height + bottom_padding; row < h; ++row)
                memset(buffer + row * w, 0, left_padding + width + right_padding);

            delete[] temp;
        }

        void FontGlyph::render_on_bitmap_unique(const FontAtlas& font_, unsigned char* buffer, int w, int h) const
        {
            font_.font_->make_bitmap(buffer, box_.width(), box_.height(), font_.scale_, index_);
        }

        FontAtlas::FontAtlas(const FontFaceRef& font_, FontOptions options, const imp::utf::string& common_buffer)
            : font_(font_)
            , options_(options)
        {
            common_dim_.cpp = 1;

            scale_ = font_->scale_for_pixel_height(options.font_size);
            int descent, linegap;
            font_->get_vertical_metrics(ascent_, descent, linegap);
            line_height_ = ascent_ - descent + linegap;

            std::vector<imp::utf::codepoint> codepoints;
            auto iter = common_buffer.begin();
            while (iter)
            {
                imp::utf::codepoint codepoint = iter.advance();
                if (codepoint == ' ')
                    continue;
                
                if (glyphs_.find(codepoint) != glyphs_.end())
                    continue;
                
                int g = font_->find_glyph_index(codepoint);
                if (!g)
                    continue;
                
                FontGlyph glyph(*this, g, scale_, common_dim_.w);
                common_dim_.w += glyph.box_.width() + 2;
                if (glyph.box_.height() > common_dim_.h)
                    common_dim_.h = glyph.box_.height();
                
                glyphs_.emplace(codepoint, std::move(glyph));
                codepoints.push_back(codepoint);
            }

            common_dim_.h += 2;
            if (common_dim_.w > 0)
            {
                unsigned char* common_buf = common_dim_.pxnew();
                for (imp::utf::codepoint codepoint : codepoints)
                {
                    auto it = glyphs_.find(codepoint);
                    it->second.render_on_bitmap_shared(*this, common_buf + it->second.buffer_pos_, common_dim_.w, common_dim_.h, 1, 1, 1, 1);
                }

                graphics::Image common_image(common_buf, common_dim_);
                common_texture_ = graphics::BindlessTextureRef(graphics::load_bindless_texture_2d(common_image, options.auto_generate_mipmaps));
                common_texture_->texture().set_parameter(GL_TEXTURE_MIN_FILTER, options.min_filter);
                common_texture_->texture().set_parameter(GL_TEXTURE_MAG_FILTER, options.mag_filter);
                common_texture_->set_and_use_handle();
                for (auto& [codepoint, glyph] : glyphs_)
                    glyph.texture_ = common_texture_;
            }

            int space_advance_width, space_left_bearing;
            font_->get_codepoint_horizontal_metrics(imp::utf::codepoint(' '), space_advance_width, space_left_bearing);
            space_advance_width_ = space_advance_width * scale_;
        }

        const FontFaceRef& FontAtlas::font_face() const
        {
            return font_;
        }

        bool FontAtlas::cache(imp::utf::codepoint codepoint) const
        {
            if (glyphs_.find(codepoint) != glyphs_.end())
                return true;

            int index = font_->find_glyph_index(codepoint);
            if (!index)
                return false;

            FontGlyph glyph(*this, index, scale_, -1);
            graphics::ImageDimensions dim = { glyph.box_.width(), glyph.box_.height(), 1 };
            unsigned char* bmp = dim.pxnew();
            glyph.render_on_bitmap_unique(*this, bmp, dim.w, dim.h);

            graphics::Image image(bmp, dim);
            graphics::BindlessTexture texture = load_bindless_texture_2d(image, options_.auto_generate_mipmaps);
            texture.texture().set_parameter(GL_TEXTURE_MIN_FILTER, options_.min_filter);
            texture.texture().set_parameter(GL_TEXTURE_MAG_FILTER, options_.mag_filter);
            texture.set_and_use_handle();
            glyph.texture_ = graphics::BindlessTextureRef(std::move(texture));
            glyphs_.emplace(codepoint, std::move(glyph));
            return true;
        }

        void FontAtlas::cache_all(const FontAtlas& other) const
        {
            for (const auto& [codepoint, glyph] : other.glyphs_)
                cache(codepoint);
        }

        const FontGlyph& FontAtlas::get_glyph(imp::utf::codepoint codepoint) const
        {
            auto it = glyphs_.find(codepoint);
            if (it != glyphs_.end())
                return it->second;
            else
                throw Error(ErrorCode::UncachedGlyph);
        }

        int FontAtlas::get_glyph_index(imp::utf::codepoint codepoint) const
        {
            auto it = glyphs_.find(codepoint);
            if (it != glyphs_.end())
                return it->second.index_;
            else
                return font_->find_glyph_index(codepoint);
        }

        bool FontAtlas::supports(imp::utf::codepoint codepoint) const
        {
            if (glyphs_.find(codepoint) != glyphs_.end())
                return true;
            return font_->find_glyph_index(codepoint) != 0;
        }

        float FontAtlas::kerning_of(imp::utf::codepoint c1, imp::utf::codepoint c2) const
        {
            return font_->get_kerning(c1, c2) * scale_;
        }

        float FontAtlas::line_height() const
        {
            return line_height_ * scale_;
        }

        float FontAtlas::get_ascent() const
        {
            return ascent_ * scale_;
        }

        math::UVRect FontAtlas::uvs(const FontGlyph& glyph) const
        {
            if (glyph.buffer_pos_ != size_t(-1)) [[likely]]
            {
                return {
                    .x1 = float(glyph.buffer_pos_ + 1) / common_dim_.w,
                    .x2 = float(glyph.buffer_pos_ + 1 + glyph.box_.width()) / common_dim_.w,
                    .y1 = 0.0f,
                    .y2 = float(glyph.box_.height()) / common_dim_.h
                };
            }
            else
                return {};
        }

        float FontAtlas::get_scale() const
        {
            return scale_;
        }

        float FontAtlas::get_scaled_space_advance_width() const
        {
            return space_advance_width_;
        }
    }
}
