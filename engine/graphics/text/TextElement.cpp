#include "TextElement.h"

#include "graphics/text/TextGlyph.h"

#include "core/algorithms/TaggedTextParser.h"
#include "core/algorithms/STLUtils.h"
#include "core/algorithms/Regex.h"

#include "core/context/rendering/Fonts.h"

#include "registries/Loader.h"

namespace oly::rendering
{
	template<typename F>
	auto visit_font(const Font::Variant& font, F&& func)
	{
		return font.visit(
			[&func](const auto& font) { return font.get().visit(std::forward<F>(func)); },
			func,
			func
		);
	}

	template<typename F>
	auto visit_fonts(const Font::Variant& f1, const Font::Variant& f2, F&& func)
	{
		return visit_font(f1, [&f2, &func](const auto& f1) { return visit_font(f2, [&f1, &func](const auto& f2) { return func(f1, f2); }); });
	}

	void Font::apply_style(FontStyle style)
	{
		if (auto font = f.safe_get<FontSelection>())
			font->style |= style;
	}

	void Font::unapply_style(FontStyle style)
	{
		if (auto font = f.safe_get<FontSelection>())
			font->style &= ~style;
	}

	void Font::reset_style()
	{
		if (auto font = f.safe_get<FontSelection>())
			font->style = FontStyle::REGULAR();
	}

	float Font::line_height() const
	{
		return visit_font(f, [](const auto& font) { return font->line_height(); });
	}

	static bool same_font(const auto& f1, const auto& f2)
	{
		if constexpr (std::is_same_v<std::decay_t<decltype(f1)>, std::decay_t<decltype(f2)>>)
			return f1 == f2;
		else
			return false;
	}

	bool Font::operator==(const FontAtlasRef& font) const
	{
		return visit_font(f, [&font](const auto& f) { return same_font(f, font); });
	}

	bool Font::operator==(const RasterFontRef& font) const
	{
		return visit_font(f, [&font](const auto& f) { return same_font(f, font); });
	}

	bool Font::operator==(const FontSelection& font) const
	{
		return visit_font(f, [&font](const auto& f) { return font.get().visit([&f](const auto& font) { return same_font(f, font); }); });
	}

	bool Font::operator==(const Font& other) const
	{
		return visit_fonts(f, other.f, [](const auto& f1, const auto& f2) { return same_font(f1, f2); });
	}

	bool Font::adj_compat(const Font& other) const
	{
		return visit_fonts(f, other.f, [](const auto& f1, const auto& f2) {
			if constexpr (visiting_class_is<decltype(f1), FontAtlasRef>)
			{
				if constexpr (visiting_class_is<decltype(f2), FontAtlasRef>)
					return f1->font_face() == f2->font_face();
				else
					return false;
			}
			else if constexpr (visiting_class_is<decltype(f1), RasterFontRef>)
			{
				if constexpr (visiting_class_is<decltype(f2), RasterFontRef>)
					return f1 == f2;
				else
					return false;
			}
			else
				return false;
			});
	}

	bool Font::support(utf::Codepoint c) const
	{
		return visit_font(f, [c](const auto& f) {
			if constexpr (visiting_class_is<decltype(f), FontAtlasRef>)
				return f->cache(c);
			else
				return f->supports(c);
			});
	}

	void Font::set_glyph(TextGlyph& glyph, utf::Codepoint c, glm::vec2 pos, glm::vec2 scale) const
	{
		visit_font(f, [&glyph, c, pos, scale](const auto& f) { glyph.set_glyph(*f, f->get_glyph(c), pos, scale); });
	}

	float Font::advance_width(utf::Codepoint c, utf::Codepoint next_codepoint) const
	{
		float adv = 0.0f;
		if (c != utf::Codepoint(' '))
			adv = visit_font(f, [c](const auto& font) {
				if constexpr (visiting_class_is<decltype(font), FontAtlasRef>)
					return font->get_glyph(c).advance_width() * font->get_scale();
				else
					return font->get_glyph(c).advance_width() * font->get_scale().x;
				});
		else
			adv = visit_font(f, [](const auto& f) { return f->get_scaled_space_advance_width(); });

		if (next_codepoint)
			adv += visit_font(f, [c, next_codepoint](const auto& f) { return f->kerning_of(c, next_codepoint); });

		return adv;
	}

	float TextElement::line_height() const
	{
		return font.line_height() * scale.y;
	}

	std::vector<TextElement> TextElement::expand(const TextElement& element)
	{
		std::vector<TextElement> expanded;
		expand(element, expanded);
		return expanded;
	}

	struct AttributeOverrides
	{
		bool font = false;
		bool text_color = false;
		bool adj_offset = false;
		bool scale = false;
		bool line_y_pivot = false;
		bool jitter_offset = false;

		bool all() const
		{
			return font && text_color && adj_offset && scale && line_y_pivot && jitter_offset;
		}
	};

	static std::string get_tag_field(const std::string& tag, size_t eq_pos)
	{
		return algo::to_lower(algo::trim(tag.substr(0, eq_pos)));
	}

	static std::string get_tag_value(const std::string& tag, size_t eq_pos)
	{
		return algo::trim(tag.substr(eq_pos + 1));
	}

	static void apply_style_tag(const std::string& tag, TextElement& e, AttributeOverrides& overrides)
	{
		if (tag == "b" || tag == "bold")
			e.font.apply_style(FontStyle::BOLD());
		else if (tag == "!b" || tag == "!bold")
			e.font.unapply_style(FontStyle::BOLD());
		else if (tag == "i" || tag == "italic")
			e.font.apply_style(FontStyle::ITALIC());
		else if (tag == "!i" || tag == "!italic")
			e.font.unapply_style(FontStyle::ITALIC());
		else if (tag == "regular")
			e.font.reset_style();
		else
		{
			OLY_LOG_WARNING(true, "RENDERING") << LOG.source_info.full_source() << "Unrecognized tag \"" << tag << "\"" << LOG.nl;
			return;
		}
		overrides.font = true;
	}

	static void apply_tag(std::string&& tag, TextElement& e, AttributeOverrides& overrides, std::vector<std::string>& style_tags)
	{
		size_t eq_pos = tag.find('=');
		if (eq_pos == std::string::npos)
		{
			if (!overrides.font)
				style_tags.push_back(std::move(tag));
			return;
		}

		std::string field = get_tag_field(tag, eq_pos);

		if (field == "font")
		{
			if (!overrides.font)
			{
				std::string value = get_tag_value(tag, eq_pos);
				size_t co_pos = value.find(':');
				if (co_pos == std::string::npos)
				{
					OLY_LOG_WARNING(true, "RENDERING") << LOG.source_info.full_source() << "Cannot parse font tag - missing ':'." << LOG.nl;
					return;
				}

				unsigned texture_index = 0;
				co_pos = value.rfind(':');
				if (co_pos != std::string::npos)
				{
					std::string index = value.substr(co_pos + 1);
					try
					{
						texture_index = std::stoul(index);
					}
					catch (...)
					{
						if (auto s = rendering::FontStyle::from_string(std::move(index)))
							texture_index = *s;
						else
						{
							OLY_LOG_WARNING(true, "RENDERING") << LOG.source_info.full_source() << "Cannot parse font file tag - texture index cannot be parsed." << LOG.nl;
							return;
						}
					}
					value.erase(value.begin() + co_pos, value.end());
				}

				if (value.ends_with('\"'))
					value.pop_back();
				if (value.starts_with('\"'))
					value.erase(value.begin());

				e.font = context::load_font(value, texture_index);
				overrides.font = true;
			}
		}
		else if (field == "color")
		{
			if (!overrides.text_color)
			{
				std::string value = get_tag_value(tag, eq_pos);
				if (algo::re::parse_vec4(value, e.text_color))
					overrides.text_color = true;
				else if (reg::parse_color(value, e.text_color))
					overrides.text_color = true;
			}
		}
		else if (field == "adj_offset")
		{
			if (!overrides.adj_offset)
			{
				if (algo::re::parse_float(get_tag_value(tag, eq_pos), e.adj_offset))
					overrides.adj_offset = true;
			}
		}
		else if (field == "scale")
		{
			if (!overrides.scale)
			{
				if (algo::re::parse_vec2(get_tag_value(tag, eq_pos), e.scale))
					overrides.scale = true;
			}
		}
		else if (field == "line_y_pivot")
		{
			if (!overrides.line_y_pivot)
			{
				float line_y_pivot;
				if (algo::re::parse_float(get_tag_value(tag, eq_pos), line_y_pivot))
				{
					overrides.line_y_pivot = true;
					e.line_y_pivot = line_y_pivot;
				}
			}
		}
		else if (field == "jitter_offset")
		{
			if (!overrides.jitter_offset)
			{
				if (algo::re::parse_vec2(get_tag_value(tag, eq_pos), e.jitter_offset))
					overrides.jitter_offset = true;
			}
		}
	}

	void TextElement::expand(const TextElement& element, std::vector<TextElement>& to)
	{
		algo::UTFTaggedTextParser parse(element.text);
		to.reserve(to.size() + parse.groups.size());
		for (algo::UTFTaggedTextParser::Group& group : parse.groups)
		{
			TextElement e{ .text = std::move(group.str) };

			AttributeOverrides overrides;

			std::vector<std::string> style_tags;
			while (!group.tags.empty())
			{
				utf::String tag = group.tags.top();
				group.tags.pop();
				apply_tag(tag.string(), e, overrides, style_tags);
				if (overrides.all())
					break;
			}
			for (auto tag = style_tags.rbegin(); tag != style_tags.rend(); ++tag)
				apply_style_tag(*tag, e, overrides);

			if (!overrides.font)
				e.font = element.font;
			if (!overrides.text_color)
				e.text_color = element.text_color;
			if (!overrides.adj_offset)
				e.adj_offset = element.adj_offset;
			if (!overrides.scale)
				e.scale = element.scale;
			if (!overrides.line_y_pivot)
				e.line_y_pivot = element.line_y_pivot;
			if (!overrides.jitter_offset)
				e.jitter_offset = element.jitter_offset;

			to.push_back(std::move(e));
		}
	}
}
