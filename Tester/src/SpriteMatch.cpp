#include "SpriteMatch.h"

#include <assets/Loader.h>
#include <assets/graphics/sprites/Sprites.h>

SpriteMatch::SpriteMatch()
{
	auto toml = oly::assets::load_toml("~/assets/archetypes/SpriteMatch.toml");

	transformer = oly::assets::load_transformer_2d(toml["archetype"]["transformer"]);

	sprite0.init(oly::assets::load_sprite(toml["sprite0"]));
	sprite2.init(oly::assets::load_sprite(toml["sprite2"]));

	sprite0->transformer.attach_parent(&transformer);
	sprite2->transformer.attach_parent(&transformer);
}

void SpriteMatch::draw() const
{
	sprite0->draw();
	sprite2->draw();
}

void SpriteMatch::on_tick() const
{
}
