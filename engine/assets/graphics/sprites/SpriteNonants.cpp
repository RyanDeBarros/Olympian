#include "SpriteNonants.h"

#include "assets/Loader.h"
#include "assets/graphics/sprites/Sprites.h"

namespace oly::assets
{
	rendering::SpriteNonant load_sprite_nonant(TOMLNode node, const char* source)
	{
		OLY_LOG_DEBUG(true, "ASSETS") << LOG.source_info.full_source() << "Parsing sprite nonant [" << (source ? source : "") << "]..." << LOG.nl;

		rendering::SpriteNonant nonant;

		glm::vec2 nsize{};
		parse_vec(node["nsize"], nsize);
		math::Padding offsets = parse_padding(node["offsets"]);

		if (auto sprite = node["sprite"])
			nonant.setup_nonant(load_sprite(sprite, source), nsize, offsets);
		else
			nonant.setup_nonant(nsize, offsets);

		OLY_LOG_DEBUG(true, "ASSETS") << LOG.source_info.full_source() << "...Sprite nonant [" << (source ? source : "") << "] parsed." << LOG.nl;

		return nonant;
	}
}
