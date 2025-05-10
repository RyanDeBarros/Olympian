#include "SpriteMatch.h"

namespace oly::gen
{
	SpriteMatch::SpriteMatch() :
		sprite0(reg::load_sprite(context::load_toml("assets/sprites/sprite0.toml")["sprite"])),
		sprite2(reg::load_sprite(context::load_toml("assets/sprites/sprite2.toml")["sprite"]))
	{}

	void SpriteMatch::draw(bool flush_sprites) const
	{
		sprite0.draw();
		sprite2.draw();
		if (flush_sprites)
			context::render_sprites();
	}
}
