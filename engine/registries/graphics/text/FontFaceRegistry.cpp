#include "FontFaceRegistry.h"

#include "core/util/LoggerOperators.h"
#include "registries/Loader.h"
#include "registries/graphics/text/KerningSupport.h"

namespace oly::reg
{
	void FontFaceRegistry::clear()
	{
		font_faces.clear();
	}

	rendering::FontFaceRef FontFaceRegistry::load_font_face(const ResourcePath& file)
	{
		auto it = font_faces.find(file);
		if (it != font_faces.end())
			return it->second;

		auto toml = load_toml(file.get_import_path());
		auto node = toml["font_face"];
		if (!node.as_table())
		{
			OLY_LOG_ERROR(true, "REG") << LOG.source_info.full_source() << "Cannot load font face " << file << " - missing \"font_face\" table." << LOG.nl;
			throw Error(ErrorCode::LOAD_ASSET);
		}

		if (LOG.enable.debug)
		{
			auto src = node["source"].value<std::string>();
			OLY_LOG_DEBUG(true, "REG") << LOG.source_info.full_source() << "Parsing font face [" << (src ? *src : "") << "]..." << LOG.nl;
		}

		rendering::FontFaceRef font_face(file, reg::parse_kerning(node));
		if (node["storage"].value<std::string>().value_or("discard") == "keep")
			font_faces.emplace(file, font_face);

		if (LOG.enable.debug)
		{
			auto src = node["source"].value<std::string>();
			OLY_LOG_DEBUG(true, "REG") << LOG.source_info.full_source() << "...Font face [" << (src ? *src : "") << "] parsed." << LOG.nl;
		}

		return font_face;
	}

	void FontFaceRegistry::free_font_face(const ResourcePath& file)
	{
		font_faces.erase(file);
	}
}
