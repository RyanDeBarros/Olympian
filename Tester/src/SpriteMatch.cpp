#include "SpriteMatch.h"

#include <assets/Loader.h>

SpriteMatch::SpriteMatch()
{
	auto toml = oly::assets::load_toml("~/assets/archetypes/SpriteMatch.toml");

	transformer = oly::Transformer2D::load(toml["archetype"]["transformer"]);

	sprite0.ref.init(oly::rendering::Sprite::load(toml["sprite0"]));
	sprite2.ref.init(oly::rendering::Sprite::load(toml["sprite2"]));

	sprite0->transformer.attach_parent(&transformer);
	sprite2->transformer.attach_parent(&transformer);

	sprite0.attach(this);
	sprite2.attach(this);
}

void SpriteMatch::on_tick()
{
}
