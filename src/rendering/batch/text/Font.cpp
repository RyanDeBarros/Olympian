#include "Font.h"

#include "../../Loader.h"
#include "util/IO.h"
#include "util/Logger.h"
#include "util/Errors.h"
#include "util/PixelBuffers.h"

namespace oly
{
	namespace rendering
	{
		static bool read_kern_part(const std::string& p, utf::Codepoint& k)
		{
			if (p[0] == '\\')
			{
				if (p.size() == 1)
					return false;
				if (p[1] == 'x')
					k = utf::Codepoint(std::stoi(p.substr(2, p.size() - 2), nullptr, 16));
				else if (p[1] == '\\')
					k = utf::Codepoint('\\');
				else if (p[1] == '\'')
					k = utf::Codepoint('\'');
				else if (p[1] == '"')
					k = utf::Codepoint('\"');
				else if (p[1] == '?')
					k = utf::Codepoint('\?');
				else if (p[1] == 'a')
					k = utf::Codepoint('\a');
				else if (p[1] == 'b')
					k = utf::Codepoint('\b');
				else if (p[1] == 'f')
					k = utf::Codepoint('\f');
				else if (p[1] == 'n')
					k = utf::Codepoint('\n');
				else if (p[1] == 'r')
					k = utf::Codepoint('\r');
				else if (p[1] == 't')
					k = utf::Codepoint('\t');
				else if (p[1] == 'v')
					k = utf::Codepoint('\v');
				else if (p[1] == '0')
					k = utf::Codepoint('\0');
				else
					return false;
			}
			else
				k = utf::Codepoint(p[0]);
			return true;
		}

		static bool parse_kerning_line(const std::string& p0, const std::string& p1,
			const std::string& p2, std::pair<std::pair<utf::Codepoint, utf::Codepoint>, int>& insert)
		{
			if (!read_kern_part(p0, insert.first.first))
				return false;
			if (!read_kern_part(p1, insert.first.second))
				return false;
			insert.second = std::stoi(p2);
			return true;
		}

		static void parse_kerning(const char* filepath, Kerning::Map& kerning)
		{
			std::string content = io::read_file(filepath);
			char part = 0;
			std::string p0, p1, p2;
			for (auto iter = content.begin(); iter != content.end(); ++iter)
			{
				if (utf::is_n_or_r(utf::Codepoint(*iter)))
				{
					std::pair<std::pair<utf::Codepoint, utf::Codepoint>, int> insert;
					if (parse_kerning_line(p0, p1, p2, insert))
						kerning.insert_or_assign(insert.first, insert.second);
					p0.clear();
					p1.clear();
					p2.clear();
					part = 0;
				}
				else if (utf::is_rn(utf::Codepoint(*iter), utf::Codepoint(iter + 1 != content.end() ? *(iter + 1) : 0)))
				{
					std::pair<std::pair<utf::Codepoint, utf::Codepoint>, int> insert;
					if (parse_kerning_line(p0, p1, p2, insert))
						kerning.insert_or_assign(insert.first, insert.second);
					p0.clear();
					p1.clear();
					p2.clear();
					part = 0;
					++iter;
				}
				else if (part == 0)
				{
					if (*iter == ' ' || *iter == '\t')
					{
						if (!p0.empty())
							++part;
					}
					else
						p0.push_back(*iter);
				}
				else if (part == 1)
				{
					if (*iter == ' ' || *iter == '\t')
					{
						if (!p1.empty())
							++part;
					}
					else
						p1.push_back(*iter);
				}
				else if (*iter != ' ' && *iter != '\t')
				{
					p2.push_back(*iter);
				}
			}
			if (part == 2)
			{
				std::pair<std::pair<utf::Codepoint, utf::Codepoint>, int> insert;
				if (parse_kerning_line(p0, p1, p2, insert))
					kerning.insert_or_assign(insert.first, insert.second);
			}
		}

		Kerning::Kerning(const char* kerning_file)
		{
			if (kerning_file)
				parse_kerning(kerning_file, map); // TODO use TOML array of triplets
		}

		FontFace::FontFace(const char* font_file, const char* kerning_file)
			: info{}, data(io::read_file_uc(font_file)), kerning(kerning_file)
		{
			if (!stbtt_InitFont(&info, data.data(), 0))
			{
				// TODO abstract this kind of temporary fatal/other behaviour in Logger.
				auto level = LOG.level;
				LOG.level = LOG.fatal;
				LOG << LOG.start << "Cannot init font" << LOG.endl;
				LOG.level = level;
			}
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

		FontGlyph::FontGlyph(FontAtlas& font, int index, float scale, size_t buffer_pos)
			: index(index), buffer_pos(buffer_pos)
		{
			font.font->get_glyph_horizontal_metrics(index, advance_width, left_bearing);
			int ch_x0, ch_x1, ch_y1;
			font.font->get_bitmap_box(index, scale, ch_x0, ch_x1, ch_y0, ch_y1);
			width = ch_x1 - ch_x0;
			height = ch_y1 - ch_y0;
		}

		void FontGlyph::render_on_bitmap_shared(const FontAtlas& font, unsigned char* buffer, int w, int h, int left_padding, int right_padding, int bottom_padding, int top_padding) const
		{
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
			font.font->make_bitmap(buffer, width, height, font.scale, index);
		}

		FontAtlas::FontAtlas(const std::shared_ptr<FontFace>& font, FontOptions options, utf::String common_buffer)
			: font(font), options(options)
		{
			common_dim.cpp = 1;

			scale = font->scale_for_pixel_height(options.font_size);
			font->get_vertical_metrics(ascent, descent, linegap);
			baseline = (int)roundf(ascent * scale);

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
				common_dim.w += glyph.width + size_t(2);
				if (glyph.height > common_dim.h)
					common_dim.h = glyph.height;
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
			space_width = (int)(roundf(space_advance_width * scale));
		}

		bool FontAtlas::cache(utf::Codepoint codepoint)
		{
			if (glyphs.find(codepoint) != glyphs.end())
				return true;
			int index = font->find_glyph_index(codepoint);
			if (!index) return false;

			FontGlyph glyph(*this, index, scale, -1);
			ImageDimensions dim = { glyph.width, glyph.height, 1 };
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

		bool FontAtlas::supports(utf::Codepoint codepoint) const
		{
			if (glyphs.find(codepoint) != glyphs.end())
				return true;
			return font->find_glyph_index(codepoint) != 0;
		}

		int FontAtlas::kerning_of(utf::Codepoint c1, utf::Codepoint c2, int g1, int g2, float sc) const
		{
			return (int)roundf(font->get_kerning(c1, c2, g1, g2) * scale * sc);
		}

		int FontAtlas::line_height(float line_spacing) const
		{
			return (int)roundf((ascent - descent + linegap) * scale * line_spacing);
		}

		math::Rect2D FontAtlas::uvs(const FontGlyph& glyph) const
		{
			math::Rect2D b{};
			if (glyph.buffer_pos != size_t(-1))
			{
				b.x1 = float(glyph.buffer_pos + 1) / common_dim.w;
				b.x2 = float(glyph.buffer_pos + 1 + glyph.width) / common_dim.w;
				b.y1 = 0.0f;
				b.y2 = float(glyph.height) / common_dim.h;
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
					auto _font_file = node["font file"].value<std::string>();
					if (_name && _font_file)
					{
						const std::string& name = _name.value();
						Constructor constructor;
						constructor.font_file = root_dir + _font_file.value();
						constructor.kerning_file = root_dir + node["kerning file"].value<std::string>().value_or("");
						constructors[name] = constructor;
						if (auto _init = node["init"].value<std::string>())
						{
							auto_loaded.emplace(name, std::make_shared<FontFace>(create_font_face(name)));
							if (_init.value() == "discard")
								constructors.erase(name);
						}
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
			return FontFace(constructor.font_file.c_str(), constructor.kerning_file.c_str());
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
