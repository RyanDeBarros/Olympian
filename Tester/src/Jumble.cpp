#include "Jumble.h"

Jumble::Jumble()
{
	auto toml = oly::io::load_toml("~/assets/archetypes/Jumble.toml");

	transformer = oly::Transformer2D::load(toml["archetype"]["transformer"]);

	sprite3.ref.init_toml(toml["sprite3"]);
	sprite4.ref.init_toml(toml["sprite4"]);
	sprite5.ref.init_toml(toml["sprite5"]);
	sprite1.ref.init_toml(toml["sprite1"]);
	godot_icon.ref.init_toml(toml["godot_icon"]);
	knight.ref.init_toml(toml["knight"]);
	test_text.ref.init_toml(toml["test_text"]);
	smol_text.ref.init_toml(toml["smol_text"]);
	atlased_knight.ref.init_toml(toml["atlased_knight"]);

	static auto VK1 = OLY_NEXT_VAULT_KEY;
	grass_tilemap.ref = oly::context::vault_prototype(VK1, [node = toml["grass_tilemap"]]() { return oly::rendering::TileMap::load(node); });

	nonant_panel.ref.init_toml(toml["nonant_panel"]);

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
