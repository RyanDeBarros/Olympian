#include "SpriteNonants.h"

#include "registries/Loader.h"

namespace oly::reg
{
	rendering::SpriteNonant load_sprite_nonant(const TOMLNode& node)
	{
		params::SpriteNonant params;

		params.sprite_params = sprite_params(node["sprite"]);

		parse_vec2(node, "nsize", params.nsize);

		parse_float(node, "left offset", params.offsets.x_left);
		parse_float(node, "right offset", params.offsets.x_right);
		parse_float(node, "bottom offset", params.offsets.y_bottom);
		parse_float(node, "top offset", params.offsets.y_top);

		return load_sprite_nonant(params);
	}

	rendering::SpriteNonant load_sprite_nonant(const params::SpriteNonant& params)
	{
		rendering::SpriteNonant nonant;
		nonant.setup_nonant(load_sprite(params.sprite_params), params.nsize, params.offsets.x_left, params.offsets.x_right, params.offsets.y_bottom, params.offsets.y_top);
		return nonant;
	}
}
