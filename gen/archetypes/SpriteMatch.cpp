#include "SpriteMatch.h"

namespace oly::gen
{
	SpriteMatch::Constructor::Constructor()
	{
		sprite0.local.position = { (float)450, (float)-200 };
		sprite0.texture = "textures/einstein.png";

		sprite2.local.position = { (float)-100, (float)-100 };
		sprite2.local.scale = { (float)0.2, (float)0.2 };
		{
			ShearTransformModifier2D modifier;
			sprite2.modifier = modifier;
		}
		sprite2.texture = "textures/tux.png";
	}

	SpriteMatch::SpriteMatch(Constructor c) :
		sprite0(reg::load_sprite(c.sprite0)),
		sprite2(reg::load_sprite(c.sprite2)),
		transformer(c.transformer.local, std::make_unique<TransformModifier2D>(*c.transformer.modifier))
	{
		sprite0.transformer.attach_parent(&transformer);
		sprite2.transformer.attach_parent(&transformer);
	}

	void SpriteMatch::draw(bool flush_sprites) const
	{
		sprite0.draw();
		sprite2.draw();
		if (flush_sprites)
			context::render_sprites();
	}

	void SpriteMatch::on_tick() const
	{
	}
}
