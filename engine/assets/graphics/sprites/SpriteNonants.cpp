#include "SpriteNonants.h"

#include "assets/Loader.h"
#include "assets/graphics/sprites/Sprites.h"

namespace oly::assets
{
	rendering::SpriteNonant load_sprite_nonant(TOMLNode node, const char* source)
	{
		_OLY_ENGINE_LOG_DEBUG("ASSETS") << "Parsing sprite nonant [" << (source ? source : "") << "]..." << LOG.nl;

		rendering::SpriteNonant nonant;

		glm::vec2 nsize{};
		parse_vec(node["nsize"], nsize);
		math::Padding offsets = math::Padding::load(node["offsets"]);

		if (auto sprite = node["sprite"])
			nonant.setup_nonant(load_sprite(sprite, source), nsize, offsets);
		else
			nonant.setup_nonant(nsize, offsets);

		_OLY_ENGINE_LOG_DEBUG("ASSETS") << "...Sprite nonant [" << (source ? source : "") << "] parsed." << LOG.nl;

		return nonant;
	}
}
