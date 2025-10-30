#include "Jumble.h"

#include <assets/Loader.h>
#include <assets/graphics/sprites/Sprites.h>
#include <assets/graphics/text/Paragraphs.h>
#include <assets/graphics/sprites/SpriteAtlases.h>
#include <assets/graphics/sprites/TileMaps.h>
#include <assets/graphics/sprites/SpriteNonants.h>

Jumble::Jumble()
{
	auto toml = oly::assets::load_toml("~/assets/archetypes/Jumble.toml");

	transformer = oly::assets::load_transformer_2d(toml["archetype"]["transformer"]);

	sprite3.ref.init(oly::assets::load_sprite(toml["sprite3"]));
	sprite4.ref.init(oly::assets::load_sprite(toml["sprite4"]));
	sprite5.ref.init(oly::assets::load_sprite(toml["sprite5"]));
	sprite1.ref.init(oly::assets::load_sprite(toml["sprite1"]));
	godot_icon.ref.init(oly::assets::load_sprite(toml["godot_icon"]));
	knight.ref.init(oly::assets::load_sprite(toml["knight"]));
	test_text.ref.init(oly::assets::load_paragraph(toml["test_text"]));
	smol_text.ref.init(oly::assets::load_paragraph(toml["smol_text"]));
	atlased_knight.ref.init(oly::assets::load_sprite_atlas(toml["atlased_knight"]));

	static auto VK1 = OLY_NEXT_VAULT_KEY;
	grass_tilemap.ref = oly::context::vault_prototype(VK1, [node = toml["grass_tilemap"]]() { return oly::assets::load_tilemap(node); });

	nonant_panel.ref.init(oly::assets::load_sprite_nonant(toml["nonant_panel"]));

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

	sprite3.attach(this);
	sprite4.attach(this);
	sprite5.attach(this);
	sprite1.attach(this);
	godot_icon.attach(this);
	knight.attach(this);
	atlased_knight.attach(this);
	grass_tilemap.attach(this);
	test_text.attach(this);
	nonant_panel.attach(this);
	smol_text.attach(this);
}

void Jumble::on_tick()
{
	atlased_knight.ref->on_tick();

	if (fmod(oly::TIME.now(), 1.0f) < 0.5f)
		grass_tilemap.set_z_layer(1);
	else
		grass_tilemap.set_z_layer(-1);
}
