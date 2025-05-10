#include "Jumble.h"

namespace oly::gen
{
	Jumble::Jumble() :
		sprite3(reg::load_sprite(context::load_toml("assets/sprites/sprite3.toml")["sprite"])),
		sprite4(reg::load_sprite(context::load_toml("assets/sprites/sprite4.toml")["sprite"])),
		sprite5(reg::load_sprite(context::load_toml("assets/sprites/sprite5.toml")["sprite"])),
		octagon(reg::load_ngon(context::load_toml("assets/polygonals/octagon.toml")["ngon"])),
		sprite1(reg::load_sprite(context::load_toml("assets/sprites/sprite1.toml")["sprite"])),
		godot_icon_3_0(reg::load_sprite(context::load_toml("assets/sprites/godot icon (3.0).toml")["sprite"])),
		knight(reg::load_sprite(context::load_toml("assets/sprites/knight.toml")["sprite"])),
		concave_shape(reg::load_poly_composite(context::load_toml("assets/polygonals/concave shape.toml")["poly_composite"])),
		atlased_knight(reg::load_sprite_atlas(context::load_toml("assets/sprites/atlased knight.toml")["sprite_atlas"])),
		grass_tilemap(reg::load_tilemap(context::load_toml("assets/tilemaps/grass tilemap.toml")["tilemap"])),
		test_text(reg::load_paragraph(context::load_toml("assets/paragraphs/test text.toml")["paragraph"]))
	{}

	void Jumble::draw(bool flush_text) const
	{
		sprite3.draw();
		sprite4.draw();
		sprite5.draw();
		context::render_sprites();
		octagon.draw();
		context::render_polygons();
		sprite1.draw();
		godot_icon_3_0.draw();
		knight.draw();
		context::render_sprites();
		concave_shape.draw();
		context::render_polygons();
		atlased_knight.sprite.draw();
		grass_tilemap.draw();
		context::render_sprites();
		test_text.draw();
		if (flush_text)
			context::render_text();
	}

	void Jumble::on_tick() const
	{
		atlased_knight.on_tick();
	}
}
