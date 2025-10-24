#pragma once

#include <Olympian.h>

struct Jumble : public oly::rendering::IDrawable
{
	oly::Transformer2D transformer;
	oly::rendering::Drawable<oly::rendering::Sprite> sprite3;
	oly::rendering::Drawable<oly::rendering::Sprite> sprite4;
	oly::rendering::Drawable<oly::rendering::Sprite> sprite5;
	oly::rendering::Drawable<oly::rendering::Sprite> sprite1;
	oly::rendering::Drawable<oly::rendering::Sprite> godot_icon;
	oly::rendering::Drawable<oly::rendering::Sprite> knight;
	oly::rendering::Drawable<oly::rendering::Paragraph> test_text;
	oly::rendering::Drawable<oly::rendering::Paragraph> smol_text;
	oly::rendering::Drawable<oly::rendering::SpriteAtlas> atlased_knight;
	oly::rendering::Drawable<oly::rendering::TileMap> grass_tilemap;
	oly::rendering::Drawable<oly::rendering::SpriteNonant> nonant_panel;

	Jumble();
	Jumble(const Jumble&) = default;
	Jumble(Jumble&&) = default;
	Jumble& operator=(const Jumble&) = default;
	Jumble& operator=(Jumble&&) = default;

	const oly::Transform2D& get_local() const { return transformer.get_local(); }
	oly::Transform2D& set_local() { return transformer.set_local(); }

	void on_tick();
};
