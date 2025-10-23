#include "Jumble.h"

#include <registries/Loader.h>
#include <registries/graphics/sprites/Sprites.h>
#include <registries/graphics/text/Paragraphs.h>
#include <registries/graphics/sprites/SpriteAtlases.h>
#include <registries/graphics/sprites/TileMaps.h>
#include <registries/graphics/sprites/SpriteNonants.h>

Jumble::Jumble()
{
	auto toml = oly::reg::load_toml("~/assets/archetypes/Jumble.toml");

	transformer = oly::reg::load_transformer_2d(toml["archetype"]["transformer"]);

	sprite3.init(oly::reg::load_sprite(toml["sprite3"]));
	sprite4.init(oly::reg::load_sprite(toml["sprite4"]));
	sprite5.init(oly::reg::load_sprite(toml["sprite5"]));
	sprite1.init(oly::reg::load_sprite(toml["sprite1"]));
	godot_icon.init(oly::reg::load_sprite(toml["godot_icon"]));
	knight.init(oly::reg::load_sprite(toml["knight"]));
	test_text.init(oly::reg::load_paragraph(toml["test_text"]));
	smol_text.init(oly::reg::load_paragraph(toml["smol_text"]));
	atlased_knight.init(oly::reg::load_sprite_atlas(toml["atlased_knight"]));
	grass_tilemap.init(oly::reg::load_tilemap(toml["grass_tilemap"]));
	nonant_panel.init(oly::reg::load_sprite_nonant(toml["nonant_panel"]));

	sprite3->transformer.attach_parent(&transformer);
	sprite4->transformer.attach_parent(&transformer);
	sprite5->transformer.attach_parent(&transformer);
	sprite1->transformer.attach_parent(&transformer);
	godot_icon->transformer.attach_parent(&transformer);
	knight->transformer.attach_parent(&transformer);
	test_text->set_transformer().attach_parent(&transformer);
	smol_text->set_transformer().attach_parent(&transformer);
	atlased_knight->sprite.transformer.attach_parent(&transformer);
	grass_tilemap->set_transformer().attach_parent(&transformer);
	nonant_panel->set_transformer().attach_parent(&transformer);
}

void Jumble::draw() const
{
	sprite3->draw();
	sprite4->draw();
	sprite5->draw();
	sprite1->draw();
	godot_icon->draw();
	knight->draw();
	atlased_knight->draw();
	grass_tilemap->draw();
	test_text->draw();
	nonant_panel->draw();
	smol_text->draw();
}

void Jumble::on_tick() const
{
	atlased_knight->on_tick();
}
