#include "FontFamilyRegistry.h"

#include "core/context/rendering/Fonts.h"
#include "core/util/LoggerOperators.h"
#include "core/base/Errors.h"
#include "registries/MetaSplitter.h"
#include "registries/Loader.h"

namespace oly::reg
{
	void FontFamilyRegistry::clear()
	{
		font_families.clear();
	}

	rendering::FontFamilyRef FontFamilyRegistry::load_font_family(const ResourcePath& file)
	{
		if (file.empty())
		{
			OLY_LOG_ERROR(true, "REG") << LOG.source_info.full_source() << "Filename is empty." << LOG.nl;
			throw Error(ErrorCode::LOAD_ASSET);
		}

		auto it = font_families.find(file);
		if (it != font_families.end())
			return it->second;

		OLY_LOG_DEBUG(true, "REG") << LOG.source_info.full_source() << "Parsing font family [" << file << "]..." << LOG.nl;

		if (!MetaSplitter::meta(file).has_type("font_family"))
		{
			OLY_LOG_ERROR(true, "REG") << LOG.source_info.full_source() << "Meta fields do not contain font family type." << LOG.nl;
			throw Error(ErrorCode::LOAD_ASSET);
		}

		auto table = load_toml(file);
		TOMLNode toml = (TOMLNode)table;

		rendering::FontFamilyRef font_family = REF_INIT;
		if (auto a = toml["styles"].as_array())
		{
			a->for_each([&file, &styles = font_family->styles](auto&& _node) {
				TOMLNode node = (TOMLNode)_node;

				rendering::FontStyle style = rendering::FontStyle::REGULAR();
				if (!parse_uint(node["style"], reinterpret_cast<unsigned int&>(style)))
				{
					auto _style_str = node["style"].value<std::string>();
					if (!_style_str)
					{
						OLY_LOG_WARNING(true, "REG") << LOG.source_info.full_source() << "Cannot parse \"style\" field from font family style" << LOG.nl;
						return;
					}

					std::string style_str = *_style_str;
					if (auto s = rendering::FontStyle::from_string(style_str))
						style = *s;
					else
					{
						OLY_LOG_WARNING(true, "REG") << LOG.source_info.full_source() << "\"style\" field \"" << style_str << "\" not recognized from font family style" << LOG.nl;
						return;
					}
				}

				auto _font_file = node["file"].value<std::string>();
				if (!_font_file)
				{
					OLY_LOG_WARNING(true, "REG") << LOG.source_info.full_source() << "Cannot parse \"file\" field from font family style" << LOG.nl;
					return;
				}
				ResourcePath font_file(*_font_file, file); // TODO v5 pass relative_to path when loading other files from asset loaders
				rendering::FontFamily::FontRef font;
				if (font_file.is_import_path())
				{
					auto meta = reg::MetaSplitter::meta(font_file);
					if (meta.has_type("raster_font"))
						font = context::load_raster_font(font_file);
					else
					{
						std::optional<std::string> type = meta.get_type();
						OLY_LOG_WARNING(true, "REG") << LOG.source_info.full_source() << font_file << " has unrecognized meta type: \"" << (type ? *type : "") << "\"" << LOG.nl;
						return;
					}
				}
				else
					font = context::load_font_atlas(font_file, parse_uint_or(node["atlas_index"], 0));

				styles.emplace(style, std::move(font));
				});
		}

		if (toml["storage"].value<std::string>().value_or("discard") == "keep")
			font_families.emplace(file, font_family);

		OLY_LOG_DEBUG(true, "REG") << LOG.source_info.full_source() << "...Font family [" << file << "] parsed." << LOG.nl;

		return font_family;
	}

	void FontFamilyRegistry::free_font_family(const ResourcePath& file)
	{
		font_families.erase(file);
	}
}
