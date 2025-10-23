#include "SpriteNonants.h"

#include "assets/Loader.h"
#include "assets/graphics/sprites/Sprites.h"

namespace oly::assets
{
	rendering::SpriteNonant load_sprite_nonant(TOMLNode node)
	{
		if (LOG.enable.debug)
		{
			auto src = node["source"].value<std::string>();
			OLY_LOG_DEBUG(true, "ASSETS") << LOG.source_info.full_source() << "Parsing sprite nonant [" << (src ? *src : "") << "]..." << LOG.nl;
		}

		rendering::SpriteNonant nonant;

		glm::vec2 nsize{};
		parse_vec(node["nsize"], nsize);
		math::Padding offsets = parse_padding(node["offsets"]);

		if (auto sprite = node["sprite"])
			nonant.setup_nonant(load_sprite(sprite), nsize, offsets);
		else
			nonant.setup_nonant(nsize, offsets);

		if (LOG.enable.debug)
		{
			auto src = node["source"].value<std::string>();
			OLY_LOG_DEBUG(true, "ASSETS") << LOG.source_info.full_source() << "...Sprite nonant [" << (src ? *src : "") << "] parsed." << LOG.nl;
		}

		return nonant;
	}
}
