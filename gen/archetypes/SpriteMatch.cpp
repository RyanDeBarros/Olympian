#include "SpriteMatch.h"

namespace oly::gen
{
	namespace
	{
		struct Constructor
		{
			struct
			{
				Transform2D local;
				std::unique_ptr<TransformModifier2D> modifier;
			} transformer;
			reg::params::Sprite sprite0;
			reg::params::Sprite sprite2;

			Constructor();
		};

		Constructor::Constructor()
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

		static std::unique_ptr<Constructor> _c;
		static Constructor& constructor()
		{
			if (!_c)
				_c = std::make_unique<Constructor>();
			return *_c;
		}
	}

	void SpriteMatch::free_constructor()
	{
		_c.reset();
	}

	SpriteMatch::SpriteMatch() :
		sprite0(reg::load_sprite(constructor().sprite0)),
		sprite2(reg::load_sprite(constructor().sprite2)),
		transformer(constructor().transformer.local, std::make_unique<TransformModifier2D>(*constructor().transformer.modifier))
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
