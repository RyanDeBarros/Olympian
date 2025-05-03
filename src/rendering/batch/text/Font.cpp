#include "Font.h"

#include "util/IO.h"
#include "util/Logger.h"
#include "util/Errors.h"
#include "util/PixelBuffers.h"

namespace oly
{
	namespace rendering
	{
		FontFace::FontFace(const char* font_file, Kerning&& kerning)
			: info{}, data(io::read_file_uc(font_file)), kerning(std::move(kerning))
		{
			if (!stbtt_InitFont(&info, data.data(), 0))
				LOG << LOG.begin_temp(LOG.fatal) << LOG.start << "Cannot init font" << LOG.end_temp << LOG.endl;
		}

		float FontFace::scale_for_pixel_height(float font_size) const
		{
			return stbtt_ScaleForPixelHeight(&info, font_size);
		}

		void FontFace::get_glyph_horizontal_metrics(int glyph_index, int& advance_width, int& left_bearing) const
		{
			stbtt_GetGlyphHMetrics(&info, glyph_index, &advance_width, &left_bearing);
		}

		void FontFace::get_codepoint_horizontal_metrics(utf::Codepoint codepoint, int& advance_width, int& left_bearing) const
		{
			stbtt_GetCodepointHMetrics(&info, codepoint, &advance_width, &left_bearing);
		}

		void FontFace::get_vertical_metrics(int& ascent, int& descent, int& linegap) const
		{
			stbtt_GetFontVMetrics(&info, &ascent, &descent, &linegap);
		}

		int FontFace::find_glyph_index(utf::Codepoint codepoint) const
		{
			return stbtt_FindGlyphIndex(&info, codepoint);
		}

		void FontFace::get_bitmap_box(int glyph_index, float scale, int& ch_x0, int& ch_x1, int& ch_y0, int& ch_y1) const
		{
			return stbtt_GetGlyphBitmapBox(&info, glyph_index, scale, scale, &ch_x0, &ch_y0, &ch_x1, &ch_y1);
		}

		void FontFace::make_bitmap(unsigned char* buf, int w, int h, float scale, int glyph_index) const
		{
			stbtt_MakeGlyphBitmap(&info, buf, w, h, w, scale, scale, glyph_index);
			flip_pixel_buffer(buf, w, h, 1);
		}

		int FontFace::get_kerning(utf::Codepoint c1, utf::Codepoint c2, int g1, int g2) const
		{
			if (g1 == 0)
				return 0;
			auto k = kerning.map.find({ c1, c2 });
			return k != kerning.map.end() ? k->second : stbtt_GetGlyphKernAdvance(&info, g1, g2);
		}

		int FontFace::get_kerning(utf::Codepoint c1, utf::Codepoint c2) const
		{
			int g1 = find_glyph_index(c1);
			int g2 = find_glyph_index(c2);
			return get_kerning(c1, c2, g1, g2);
		}

		FontGlyph::FontGlyph(FontAtlas& font, int index, float scale, size_t buffer_pos)
			: index(index), buffer_pos(buffer_pos)
		{
			font.font->get_glyph_horizontal_metrics(index, advance_width, left_bearing);
			font.font->get_bitmap_box(index, scale, box.x1, box.x2, box.y1, box.y2);
		}

		void FontGlyph::render_on_bitmap_shared(const FontAtlas& font, unsigned char* buffer, int w, int h, int left_padding, int right_padding, int bottom_padding, int top_padding) const
		{
			int width = box.width();
			int height = box.height();
			unsigned char* temp = new unsigned char[width * height];
			font.font->make_bitmap(temp, width, height, font.scale, index);
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

		void FontGlyph::render_on_bitmap_unique(const FontAtlas& font, unsigned char* buffer, int w, int h) const
		{
			font.font->make_bitmap(buffer, box.width(), box.height(), font.scale, index);
		}

		FontAtlas::FontAtlas(const std::shared_ptr<FontFace>& font, FontOptions options, utf::String common_buffer)
			: font(font), options(options)
		{
			common_dim.cpp = 1;

			scale = font->scale_for_pixel_height(options.font_size);
			font->get_vertical_metrics(ascent, descent, linegap);
			baseline = ascent * scale;

			std::vector<utf::Codepoint> codepoints;
			auto iter = common_buffer.begin();
			while (iter)
			{
				utf::Codepoint codepoint = iter.advance();
				if (codepoint == ' ')
					continue;
				if (glyphs.find(codepoint) != glyphs.end())
					continue;
				int g = font->find_glyph_index(codepoint);
				if (!g)
					continue;
				FontGlyph glyph(*this, g, scale, common_dim.w);
				common_dim.w += glyph.box.width() + 2;
				if (glyph.box.height() > common_dim.h)
					common_dim.h = glyph.box.height();
				glyphs.emplace(codepoint, std::move(glyph));
				codepoints.push_back(codepoint);
			}
			common_dim.h += 2;
			if (common_dim.w > 0)
			{
				unsigned char* common_buf = common_dim.pxnew();
				for (utf::Codepoint codepoint : codepoints)
				{
					auto it = glyphs.find(codepoint);
					it->second.render_on_bitmap_shared(*this, common_buf + it->second.buffer_pos, common_dim.w, common_dim.h, 1, 1, 1, 1);
				}
				Image common_image(common_buf, common_dim);
				common_texture = move_shared(load_bindless_texture_2d(common_image, options.auto_generate_mipmaps));
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, options.min_filter);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, options.mag_filter);
				common_texture->set_and_use_handle();
				for (auto& [codepoint, glyph] : glyphs)
					glyph.texture = common_texture;
			}
			int space_advance_width, space_left_bearing;
			font->get_codepoint_horizontal_metrics(utf::Codepoint(' '), space_advance_width, space_left_bearing);
			space_width = space_advance_width * scale;
		}

		bool FontAtlas::cache(utf::Codepoint codepoint)
		{
			if (glyphs.find(codepoint) != glyphs.end())
				return true;
			int index = font->find_glyph_index(codepoint);
			if (!index) return false;

			FontGlyph glyph(*this, index, scale, -1);
			ImageDimensions dim = { glyph.box.width(), glyph.box.height(), 1};
			unsigned char* bmp = dim.pxnew();
			glyph.render_on_bitmap_unique(*this, bmp, dim.w, dim.h);

			Image image(bmp, dim);
			BindlessTexture texture = load_bindless_texture_2d(image, options.auto_generate_mipmaps);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, options.min_filter);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, options.mag_filter);
			texture.set_and_use_handle();
			glyph.texture = move_shared(std::move(texture));
			glyphs.emplace(codepoint, std::move(glyph));
			return true;
		}

		void FontAtlas::cache_all(const FontAtlas& other)
		{
			for (const auto& [codepoint, glyph] : other.glyphs)
				cache(codepoint);
		}

		const FontGlyph& FontAtlas::get_glyph(utf::Codepoint codepoint) const
		{
			auto it = glyphs.find(codepoint);
			if (it != glyphs.end())
				return it->second;
			else
				throw Error(ErrorCode::UNCACHED_GLYPH);
		}

		int FontAtlas::get_glyph_index(utf::Codepoint codepoint) const
		{
			auto it = glyphs.find(codepoint);
			if (it != glyphs.end())
				return it->second.index;
			else
				return font->find_glyph_index(codepoint);
		}

		bool FontAtlas::supports(utf::Codepoint codepoint) const
		{
			if (glyphs.find(codepoint) != glyphs.end())
				return true;
			return font->find_glyph_index(codepoint) != 0;
		}

		float FontAtlas::kerning_of(utf::Codepoint c1, utf::Codepoint c2, int g1, int g2) const
		{
			return font->get_kerning(c1, c2, g1, g2) * scale;
		}

		float FontAtlas::kerning_of(utf::Codepoint c1, utf::Codepoint c2) const
		{
			return font->get_kerning(c1, c2) * scale;
		}

		float FontAtlas::line_height() const
		{
			return (ascent - descent + linegap) * scale;
		}

		float FontAtlas::get_ascent() const
		{
			return ascent * scale;
		}

		float FontAtlas::get_descent() const
		{
			return descent * scale;
		}

		float FontAtlas::get_linegap() const
		{
			return linegap * scale;
		}

		math::Rect2D FontAtlas::uvs(const FontGlyph& glyph) const
		{
			math::Rect2D b{};
			if (glyph.buffer_pos != size_t(-1))
			{
				b.x1 = float(glyph.buffer_pos + 1) / common_dim.w;
				b.x2 = float(glyph.buffer_pos + 1 + glyph.box.width()) / common_dim.w;
				b.y1 = 0.0f;
				b.y2 = float(glyph.box.height()) / common_dim.h;
			}
			else
			{
				b.x1 = 0;
				b.x2 = 1;
				b.y1 = 0;
				b.y2 = 1;
			}
			return b;
		}

		void FontFaceRegistry::load(const char* font_face_registry_file)
		{
			auto toml = assets::load_toml(font_face_registry_file);
			auto font_face_list = toml["font_face"].as_array();
			if (!font_face_list)
				return;
			std::string root_dir = io::directory_of(font_face_registry_file);
			font_face_list->for_each([this, &root_dir](auto&& node) {
				if constexpr (toml::is_table<decltype(node)>)
				{
					auto _name = node["name"].value<std::string>();
					auto _font_file = node["file"].value<std::string>();
					if (_name && _font_file)
					{
						const std::string& name = _name.value();
						Constructor constructor;
						constructor.font_file = root_dir + _font_file.value();
						if (auto kerning_arr = node["kerning"].as_array())
						{
							for (const auto& node : *kerning_arr)
							{
								if (auto triplet = node.as_array())
								{
									utf::Codepoint c1, c2;
									int k;
									static const auto parse_codepoint = [](const std::string& sc) -> utf::Codepoint {
										if (sc.size() >= 3)
										{
											std::string prefix = sc.substr(0, 2);
											if (prefix == "U+" || prefix == "0x" || prefix == "0X" || prefix == "\\u" || prefix == "\\U" || prefix == "0h")
												return utf::Codepoint(std::stoi(sc.substr(2), nullptr, 16));
											if (sc.substr(0, 3) == "&#x" && sc.ends_with(";"))
												return utf::Codepoint(std::stoi(sc.substr(3, sc.size() - 3 - 1), nullptr, 16));
										}
										else if (sc.empty())
											return utf::Codepoint(0);
										return utf::Codepoint(sc[0]);
									};
									if (auto tc1 = triplet->get_as<std::string>(0))
										c1 = parse_codepoint(tc1->get());
									else
										continue;
									if (auto tc2 = triplet->get_as<std::string>(1))
										c2 = parse_codepoint(tc2->get());
									else
										continue;
									if (auto tk = triplet->get_as<int64_t>(2))
										k = (int)tk->get();
									else
										continue;
									if (c1 && c2)
										constructor.kerning.map.emplace(std::make_pair(c1, c2), k);
								}
							}
						}
						if (auto _init = node["init"].value<std::string>())
						{
							if (_init.value() == "discard")
								auto_loaded.emplace(name, std::make_shared<FontFace>(constructor.font_file.c_str(), std::move(constructor.kerning)));
							else
							{
								auto_loaded.emplace(name, std::make_shared<FontFace>(constructor.font_file.c_str(), dupl(constructor.kerning)));
								constructors[name] = constructor;
							}
						}
						else
							constructors[name] = constructor;
					}
				}
				});
		}

		void FontFaceRegistry::clear()
		{
			constructors.clear();
			auto_loaded.clear();
		}

		FontFace FontFaceRegistry::create_font_face(const std::string& name) const
		{
			auto it = constructors.find(name);
			if (it == constructors.end())
				throw Error(ErrorCode::UNREGISTERED_FONT_FACE);
			const auto& constructor = it->second;
			return FontFace(constructor.font_file.c_str(), dupl(constructor.kerning));
		}

		std::weak_ptr<FontFace> FontFaceRegistry::ref_font_face(const std::string& name) const
		{
			auto it = auto_loaded.find(name);
			if (it == auto_loaded.end())
				throw Error(ErrorCode::UNREGISTERED_FONT_FACE);
			return it->second;
		}

		void FontFaceRegistry::delete_font_face(const std::string& name)
		{
			auto_loaded.erase(name);
		}
		
		void FontAtlasRegistry::load(const FontFaceRegistry& font_face_registry, const char* font_atlas_registry_file)
		{
			auto toml = assets::load_toml(font_atlas_registry_file);
			auto font_atlas_list = toml["font_atlas"].as_array();
			if (!font_atlas_list)
				return;
			font_atlas_list->for_each([this, &font_face_registry](auto&& node) {
				if constexpr (toml::is_table<decltype(node)>)
				{
					auto _name = node["name"].value<std::string>();
					auto _font_face = node["font face"].value<std::string>();
					auto _font_size_double = node["font size"].value<double>();
					auto _font_size_int = node["font size"].value<int64_t>();
					if (_name && _font_face && (_font_size_double || _font_size_int))
					{
						const std::string& name = _name.value();
						Constructor constructor;
						constructor.font_face_name = _font_face.value();
						constructor.options.font_size = _font_size_double ? (float)_font_size_double.value() : (float)_font_size_int.value();
						assets::parse_min_filter(node, "min filter", constructor.options.min_filter);
						assets::parse_mag_filter(node, "mag filter", constructor.options.mag_filter);
						constructor.options.auto_generate_mipmaps = node["generate mipmaps"].value<bool>().value_or(false);

						auto _common_buffer_preset = node["common buffer preset"].value<std::string>();
						if (_common_buffer_preset)
						{
							const std::string& common_buffer_preset = _common_buffer_preset.value();
							if (common_buffer_preset == "common")
								constructor.common_buffer = glyphs::COMMON;
							else if (common_buffer_preset == "alpha numeric")
								constructor.common_buffer = glyphs::ALPHA_NUMERIC;
							else if (common_buffer_preset == "numeric")
								constructor.common_buffer = glyphs::NUMERIC;
							else if (common_buffer_preset == "alphabet")
								constructor.common_buffer = glyphs::ALPHABET;
							else if (common_buffer_preset == "alphabet lowercase")
								constructor.common_buffer = glyphs::ALPHABET_LOWERCASE;
							else if (common_buffer_preset == "alphabet uppercase")
								constructor.common_buffer = glyphs::ALPHABET_UPPERCASE;
						}
						auto _common_buffer = node["common buffer"].value<std::string>();
						if (_common_buffer)
							constructor.common_buffer = _common_buffer.value();

						constructors[name] = constructor;
						if (auto _init = node["init"].value<std::string>())
						{
							auto_loaded.emplace(name, std::make_shared<FontAtlas>(create_font_atlas(font_face_registry, name)));
							if (_init.value() == "discard")
								constructors.erase(name);
						}
					}
				}
				});
		}
		
		void FontAtlasRegistry::clear()
		{
			constructors.clear();
			auto_loaded.clear();
		}

		FontAtlas FontAtlasRegistry::create_font_atlas(const FontFaceRegistry& font_face_registry, const std::string& name) const
		{
			auto it = constructors.find(name);
			if (it == constructors.end())
				throw Error(ErrorCode::UNREGISTERED_FONT_ATLAS);
			const auto& constructor = it->second;

			return FontAtlas(font_face_registry.ref_font_face(constructor.font_face_name).lock(), constructor.options, constructor.common_buffer);
		}
		
		std::weak_ptr<FontAtlas> FontAtlasRegistry::ref_font_atlas(const std::string& name) const
		{
			auto it = auto_loaded.find(name);
			if (it == auto_loaded.end())
				throw Error(ErrorCode::UNREGISTERED_FONT_ATLAS);
			return it->second;
		}
		
		void FontAtlasRegistry::delete_font_atlas(const std::string& name)
		{
			auto_loaded.erase(name);
		}
}
}
