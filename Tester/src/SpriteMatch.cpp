#include "SpriteMatch.h"

SpriteMatch::SpriteMatch()
{
	auto toml = oly::io::load_toml("~/assets/archetypes/SpriteMatch.toml");

	transformer = oly::Transformer2D::load(toml["archetype"]["transformer"]);

	sprite0.ref.init_toml(toml["sprite0"]);
	sprite2.ref.init_toml(toml["sprite2"]);

	sprite0->transformer.attach_parent(&transformer);
	sprite2->transformer.attach_parent(&transformer);

	sprite0.attach(this);
	sprite2.attach(this);
}
