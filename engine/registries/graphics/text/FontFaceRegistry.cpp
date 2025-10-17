#include "FontFaceRegistry.h"

#include "core/context/Context.h"
#include "core/util/Logger.h"
#include "registries/Loader.h"
#include "registries/graphics/text/KerningSupport.h"

namespace oly::reg
{
	void FontFaceRegistry::clear()
	{
		font_faces.clear();
	}

	rendering::FontFaceRef FontFaceRegistry::load_font_face(const std::string& file)
	{
		auto it = font_faces.find(file);
		if (it != font_faces.end())
			return it->second;

		auto toml = load_toml(context::resource_file(file + ".oly"));
		auto node = toml["font_face"];
		if (!node.as_table())
		{
			OLY_LOG_ERROR(true, "REG") << LOG.source_info.full_source() << "Cannot load font face \"" << file << "\" - missing \"font_face\" table." << LOG.nl;
			throw Error(ErrorCode::LOAD_ASSET);
		}

		if (LOG.enable.debug)
		{
			auto src = node["source"].value<std::string>();
			OLY_LOG_DEBUG(true, "REG") << LOG.source_info.full_source() << "Parsing font face [" << (src ? *src : "") << "]..." << LOG.nl;
		}

		rendering::FontFaceRef font_face(context::resource_file(file).c_str(), reg::parse_kerning(node));
		if (node["storage"].value<std::string>().value_or("discard") == "keep")
			font_faces.emplace(file, font_face);

		if (LOG.enable.debug)
		{
			auto src = node["source"].value<std::string>();
			OLY_LOG_DEBUG(true, "REG") << LOG.source_info.full_source() << "...Font face [" << (src ? *src : "") << "] parsed." << LOG.nl;
		}

		return font_face;
	}

	void FontFaceRegistry::free_font_face(const std::string& file)
	{
		font_faces.erase(file);
	}
}
