#include "SpriteNonants.h"

#include "registries/Loader.h"

namespace oly::reg
{
	rendering::SpriteNonant load_sprite_nonant(TOMLNode node)
	{
		if (LOG.enable.debug)
		{
			auto src = node["source"].value<std::string>();
			OLY_LOG_DEBUG(true, "REG") << LOG.source_info.full_source() << "Parsing sprite nonant [" << (src ? *src : "") << "]." << LOG.nl;
		}

		params::SpriteNonant params;

		params.sprite_params = sprite_params(node["sprite"]);

		parse_vec(node["nsize"], params.nsize);
		params.offsets = parse_padding(node["offsets"]);

		if (LOG.enable.debug)
		{
			auto src = node["source"].value<std::string>();
			OLY_LOG_DEBUG(true, "REG") << LOG.source_info.full_source() << "Sprite nonant [" << (src ? *src : "") << "] parsed." << LOG.nl;
		}

		return load_sprite_nonant(params);
	}

	rendering::SpriteNonant load_sprite_nonant(const params::SpriteNonant& params)
	{
		rendering::SpriteNonant nonant;
		nonant.setup_nonant(load_sprite(params.sprite_params), params.nsize, params.offsets);
		return nonant;
	}
}
