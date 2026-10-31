#include "SpriteMatch.h"

SpriteMatch::SpriteMatch()
{
    auto toml = oly::io::load_toml("@/assets/archetypes/SpriteMatch.toml");
    auto table = (oly::TOMLNode)toml;

    transformer = oly::Transformer2D::load(table["archetype"]["transformer"]);

    sprite0.ref.init_toml(table["sprite0"]);
    sprite2.ref.init_toml(table["sprite2"]);

    sprite0->transformer.attach_parent(&transformer);
    sprite2->transformer.attach_parent(&transformer);

    sprite0.attach(this);
    sprite2.attach(this);
}

const oly::Transform2D& SpriteMatch::get_local() const
{
    return transformer.get_local();
}

oly::Transform2D& SpriteMatch::set_local()
{
    return transformer.set_local();
}
