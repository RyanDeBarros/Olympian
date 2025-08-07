#include "SpriteMatch.h"

#include "registries/Loader.h"
#include "registries/graphics/primitives/Sprites.h"

namespace oly::gen
{
    SpriteMatch::SpriteMatch()
    {
        {
            reg::params::Transformer2D params;

            transformer = reg::load_transformer_2d(params);
        }

        {
            reg::params::Sprite params;
			params.local.position = { (float)450, (float)-200 };
			params.texture = "textures/einstein.png";

            sprite0.init(reg::load_sprite(params));
        }

        {
            reg::params::Sprite params;
			params.local.position = { (float)-100, (float)-100 };
			params.local.scale = { (float)0.2, (float)0.2 };
		{
			ShearTransformModifier2D modifier;
			params.modifier = modifier;
		}
			params.texture = "textures/tux.png";

            sprite2.init(reg::load_sprite(params));
        }

		sprite0->transformer.attach_parent(&transformer);
		sprite2->transformer.attach_parent(&transformer);
    }


    void SpriteMatch::draw(bool flush_sprites) const
    {
		sprite0->draw();
		sprite2->draw();
		if (flush_sprites)
			context::render_sprites();
	}


    void SpriteMatch::on_tick() const
    {
	}
}
