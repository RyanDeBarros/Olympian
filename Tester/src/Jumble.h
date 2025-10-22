#pragma once

#include <Olympian.h>

struct Jumble
{
	oly::Transformer2D transformer;
	oly::rendering::SpriteRef sprite3;
	oly::rendering::SpriteRef sprite4;
	oly::rendering::SpriteRef sprite5;
	oly::rendering::SpriteRef sprite1;
	oly::rendering::SpriteRef godot_icon;
	oly::rendering::SpriteRef knight;
	oly::rendering::ParagraphRef test_text;
	oly::rendering::ParagraphRef smol_text;
	oly::rendering::SpriteAtlasRef atlased_knight;
	oly::rendering::TileMapRef grass_tilemap;
	oly::rendering::SpriteNonantRef nonant_panel;

	Jumble();
	Jumble(const Jumble&) = default;
	Jumble(Jumble&&) = default;
	Jumble& operator=(const Jumble&) = default;
	Jumble& operator=(Jumble&&) = default;

	const oly::Transform2D& get_local() const { return transformer.get_local(); }
	oly::Transform2D& set_local() { return transformer.set_local(); }

	void draw() const;

	void on_tick() const;
};
