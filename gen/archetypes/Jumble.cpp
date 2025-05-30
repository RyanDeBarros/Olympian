#include "Jumble.h"

namespace oly::gen
{
	namespace
	{
		struct Constructor
		{
			struct
			{
				Transform2D local;
				std::unique_ptr<TransformModifier2D> modifier;
			} transformer;
			reg::params::Sprite sprite3;
			reg::params::Sprite sprite4;
			reg::params::Sprite sprite5;
			reg::params::Sprite sprite1;
			reg::params::Sprite godot_icon;
			reg::params::Sprite knight;
			reg::params::PolyComposite concave_shape;
			reg::params::NGon octagon;
			reg::params::Paragraph test_text;
			reg::params::SpriteAtlas atlased_knight;
			reg::params::TileMap grass_tilemap;
			reg::params::SpriteNonant nonant_panel;

			Constructor();
		};

		Constructor::Constructor()
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
			sprite1.modulation = rendering::ModulationRect{
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

			nonant_panel.sprite_params.local.scale = { (float)10.0, (float)10.0 };
			nonant_panel.sprite_params.texture = "textures/panel.png";
			nonant_panel.sprite_params.modulation = rendering::ModulationRect{
				glm::vec4{ (float)1.0, (float)0.3, (float)0.3, (float)1.0 },
				glm::vec4{ (float)0.3, (float)0.3, (float)0.3, (float)1.0 },
				glm::vec4{ (float)0.3, (float)0.3, (float)1.0, (float)1.0 },
				glm::vec4{ (float)0.3, (float)0.3, (float)0.3, (float)1.0 }
			};
			nonant_panel.nsize = { (float)128, (float)0 };
			nonant_panel.offsets.x_left = (float)6;
			nonant_panel.offsets.x_right = (float)6;
			nonant_panel.offsets.y_bottom = (float)6;
			nonant_panel.offsets.y_top = (float)6;
		}

		static std::unique_ptr<Constructor> _c;
		static Constructor& constructor()
		{
			if (!_c)
				_c = std::make_unique<Constructor>();
			return *_c;
		}
	}

	void Jumble::free_constructor()
	{
		_c.reset();
	}

	Jumble::Jumble() :
		sprite3(reg::load_sprite(std::move(constructor().sprite3))),
		sprite4(reg::load_sprite(std::move(constructor().sprite4))),
		sprite5(reg::load_sprite(std::move(constructor().sprite5))),
		sprite1(reg::load_sprite(std::move(constructor().sprite1))),
		godot_icon(reg::load_sprite(std::move(constructor().godot_icon))),
		knight(reg::load_sprite(std::move(constructor().knight))),
		concave_shape(reg::load_poly_composite(std::move(constructor().concave_shape))),
		octagon(reg::load_ngon(std::move(constructor().octagon))),
		test_text(reg::load_paragraph(std::move(constructor().test_text))),
		atlased_knight(reg::load_sprite_atlas(std::move(constructor().atlased_knight))),
		grass_tilemap(reg::load_tilemap(std::move(constructor().grass_tilemap))),
		nonant_panel(reg::load_sprite_nonant(std::move(constructor().nonant_panel))),
		transformer(constructor().transformer.local, std::make_unique<TransformModifier2D>(*constructor().transformer.modifier))
	{
		sprite3.transformer.attach_parent(&transformer);
		sprite4.transformer.attach_parent(&transformer);
		sprite5.transformer.attach_parent(&transformer);
		sprite1.transformer.attach_parent(&transformer);
		godot_icon.transformer.attach_parent(&transformer);
		knight.transformer.attach_parent(&transformer);
		concave_shape.transformer.attach_parent(&transformer);
		octagon.transformer.attach_parent(&transformer);
		test_text.transformer.attach_parent(&transformer);
		grass_tilemap.transformer.attach_parent(&transformer);
		nonant_panel.transformer.attach_parent(&transformer);
		atlased_knight.sprite.transformer.attach_parent(&transformer);
		free_constructor();
	}

	void Jumble::draw(bool flush_sprites) const
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
		context::render_text();
		nonant_panel.draw();
		if (flush_sprites)
			context::render_sprites();
	}

	void Jumble::on_tick() const
	{
		atlased_knight.on_tick();
	}
}
