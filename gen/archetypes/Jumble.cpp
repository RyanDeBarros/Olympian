#include "Jumble.h"

namespace oly::gen
{
	Jumble::Constructor::Constructor()
	{
		sprite3.local.position = { (float)400, (float)400 };
		sprite3.local.scale = { (float)0.5, (float)0.5 };
		sprite3.texture = "textures/serotonin.gif";
		{
			graphics::AnimFrameFormat frame_format;
			frame_format.num_frames = (GLuint)59;
			frame_format.delay_seconds = (float)-0.05;
			sprite3.frame_format = frame_format;
		}

		sprite4.local.position = { (float)-500, (float)300 };
		sprite4.local.scale = { (float)0.1, (float)0.1 };
		sprite4.texture = "textures/tux.png";
		sprite4.texture_index = (unsigned int)1;

		sprite5.local.position = { (float)-400, (float)300 };
		sprite5.local.scale = { (float)0.1, (float)0.1 };
		sprite5.texture = "textures/tux.png";

		sprite1.texture = "textures/einstein.png";
		sprite1.modulation = rendering::SpriteBatch::Modulation{
			glm::vec4{ (float)1.0, (float)1.0, (float)0.2, (float)0.7 },
			glm::vec4{ (float)0.2, (float)1.0, (float)1.0, (float)0.7 },
			glm::vec4{ (float)1.0, (float)0.2, (float)1.0, (float)0.7 },
			glm::vec4{ (float)0.5, (float)0.5, (float)0.5, (float)0.7 }
		};

		godot_icon.local.position = { (float)-300, (float)-200 };
		godot_icon.local.rotation = (float)0.3;
		godot_icon.local.scale = { (float)1.0, (float)1.0 };
		godot_icon.texture = "textures/godot.svg";
		godot_icon.svg_scale = (float)3.0;

		knight.local.position = { (float)100, (float)-300 };
		knight.local.scale = { (float)20, (float)20 };
		knight.texture = "textures/knight.png";
		{
			reg::params::Sprite::AutoFrameFormat frame_format;
			knight.frame_format = frame_format;
		}

		concave_shape.local.position = { (float)-200, (float)200 };
		concave_shape.local.scale = { (float)60, (float)60 };
		{
			reg::params::PolyComposite::ConvexDecompositionMethod method;
			method.points.reserve(10);
			method.points.push_back({ (float)-4, (float)0 });
			method.points.push_back({ (float)-2, (float)-2 });
			method.points.push_back({ (float)0, (float)-2 });
			method.points.push_back({ (float)2, (float)-1 });
			method.points.push_back({ (float)4, (float)1 });
			method.points.push_back({ (float)2, (float)3 });
			method.points.push_back({ (float)1, (float)3 });
			method.points.push_back({ (float)-1, (float)0 });
			method.points.push_back({ (float)-3, (float)1 });
			method.points.push_back({ (float)-3, (float)2 });
			concave_shape.method = method;
		}

		octagon.local.position = { (float)300, (float)200 };
		octagon.local.scale = { (float)200, (float)200 };
		octagon.bordered = true;
		octagon.ngon_base.points.reserve(8);
		octagon.ngon_base.points.push_back({ (float)1, (float)0 });
		octagon.ngon_base.points.push_back({ (float)0.707, (float)0.707 });
		octagon.ngon_base.points.push_back({ (float)0, (float)1 });
		octagon.ngon_base.points.push_back({ (float)-0.707, (float)0.707 });
		octagon.ngon_base.points.push_back({ (float)-1, (float)0 });
		octagon.ngon_base.points.push_back({ (float)-0.707, (float)-0.707 });
		octagon.ngon_base.points.push_back({ (float)0, (float)-1 });
		octagon.ngon_base.points.push_back({ (float)0.707, (float)-0.707 });
		octagon.ngon_base.fill_colors.reserve(1);
		octagon.ngon_base.fill_colors.push_back({ (float)0.0, (float)1.0, (float)0.0, (float)0.7 });
		octagon.ngon_base.border_colors.reserve(8);
		octagon.ngon_base.border_colors.push_back({ (float)0.25, (float)0.0, (float)0.5, (float)1.0 });
		octagon.ngon_base.border_colors.push_back({ (float)0.0, (float)0.25, (float)0.25, (float)1.0 });
		octagon.ngon_base.border_colors.push_back({ (float)0.25, (float)0.5, (float)0.0, (float)1.0 });
		octagon.ngon_base.border_colors.push_back({ (float)0.5, (float)0.75, (float)0.25, (float)1.0 });
		octagon.ngon_base.border_colors.push_back({ (float)0.75, (float)1.0, (float)0.5, (float)1.0 });
		octagon.ngon_base.border_colors.push_back({ (float)1.0, (float)0.75, (float)0.75, (float)1.0 });
		octagon.ngon_base.border_colors.push_back({ (float)0.75, (float)0.5, (float)1.0, (float)1.0 });
		octagon.ngon_base.border_colors.push_back({ (float)0.5, (float)0.25, (float)0.75, (float)1.0 });
		octagon.ngon_base.border_width = (float)0.05;

		test_text.local.position = { (float)-400, (float)400 };
		test_text.local.scale = { (float)0.8, (float)0.8 };
		test_text.font_atlas = "fonts/Roboto-Regular.ttf";
		test_text.text = "rgb x\txx  x.\nabcd !!!\r\n\n123478s\nHex";
		test_text.draw_bkg = true;
		test_text.bkg_color = { (float)0.5, (float)0.4, (float)0.2, (float)0.8 };
		test_text.text_color = { (float)0.0, (float)0.0, (float)0.0, (float)1.0 };
		test_text.glyph_colors.reserve(3);
		test_text.glyph_colors.push_back({ 0, { (float)1.0, (float)0.0, (float)0.0, (float)1.0 } });
		test_text.glyph_colors.push_back({ 1, { (float)0.0, (float)1.0, (float)0.0, (float)1.0 } });
		test_text.glyph_colors.push_back({ 2, { (float)0.0, (float)0.0, (float)1.0, (float)1.0 } });
		test_text.format.pivot = { (float)0.0, (float)1.0 };
		test_text.format.line_spacing = (float)1.5;
		test_text.format.linebreak_spacing = (float)2.0;
		test_text.format.min_size = { (float)800, (float)800 };
		test_text.format.padding = { (float)50, (float)50 };
		test_text.format.horizontal_alignment = rendering::ParagraphFormat::HorizontalAlignment::CENTER;
		test_text.format.vertical_alignment = rendering::ParagraphFormat::VerticalAlignment::MIDDLE;

		atlased_knight.sprite_params.local.position = { (float)300, (float)-200 };
		atlased_knight.sprite_params.local.scale = { (float)20, (float)20 };
		atlased_knight.sprite_params.texture = "textures/knight.png";
		atlased_knight.sprite_params.texture_index = (unsigned int)1;
		{
			reg::params::SpriteAtlas::Frame frame{ .rows = (GLuint)2, .cols = (GLuint)3, .delay_seconds = (float)0.1 };
			atlased_knight.frame = frame;
		}

		grass_tilemap.local.scale = { (float)100, (float)100 };
		{
			reg::params::TileMap::Layer layer;
			layer.tileset = "assets/grass tileset.toml";
			layer.z = (size_t)0;
			layer.tiles.reserve(13);
			layer.tiles.push_back({ (float)-1, (float)-1 });
			layer.tiles.push_back({ (float)0, (float)-1 });
			layer.tiles.push_back({ (float)1, (float)-1 });
			layer.tiles.push_back({ (float)-1, (float)0 });
			layer.tiles.push_back({ (float)1, (float)0 });
			layer.tiles.push_back({ (float)2, (float)0 });
			layer.tiles.push_back({ (float)-1, (float)1 });
			layer.tiles.push_back({ (float)0, (float)1 });
			layer.tiles.push_back({ (float)1, (float)1 });
			layer.tiles.push_back({ (float)2, (float)2 });
			layer.tiles.push_back({ (float)3, (float)2 });
			layer.tiles.push_back({ (float)2, (float)3 });
			layer.tiles.push_back({ (float)3, (float)3 });
			grass_tilemap.layers.push_back(std::move(layer));
		}
	}

	Jumble::Jumble(Constructor c) :
		sprite3(reg::load_sprite(c.sprite3)),
		sprite4(reg::load_sprite(c.sprite4)),
		sprite5(reg::load_sprite(c.sprite5)),
		sprite1(reg::load_sprite(c.sprite1)),
		godot_icon(reg::load_sprite(c.godot_icon)),
		knight(reg::load_sprite(c.knight)),
		concave_shape(reg::load_poly_composite(c.concave_shape)),
		octagon(reg::load_ngon(c.octagon)),
		test_text(reg::load_paragraph(c.test_text)),
		atlased_knight(reg::load_sprite_atlas(c.atlased_knight)),
		grass_tilemap(reg::load_tilemap(c.grass_tilemap))
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
		godot_icon.draw();
		knight.draw();
		context::render_sprites();
		concave_shape.draw();
		context::render_polygons();
		atlased_knight.draw();
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
