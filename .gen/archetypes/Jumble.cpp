#include "Jumble.h"

#include "registries/Loader.h"
#include "registries/graphics/primitives/Sprites.h"
#include "registries/graphics/primitives/Polygons.h"
#include "registries/graphics/text/Paragraphs.h"
#include "registries/graphics/extensions/SpriteAtlases.h"
#include "registries/graphics/extensions/TileMaps.h"
#include "registries/graphics/extensions/SpriteNonants.h"

namespace oly::gen
{
    Jumble::Jumble()
    {
        {
            reg::params::Transformer2D params;

            transformer = reg::load_transformer_2d(params);
        }

        {
            reg::params::Sprite params;
			params.local.position = { (float)400, (float)400 };
			params.local.scale = { (float)0.5, (float)0.5 };
			params.texture = "textures/serotonin.gif";
			{
				graphics::AnimFrameFormat frame_format;
				frame_format.num_frames = (GLuint)59;
				frame_format.delay_seconds = (float)-0.05;
				params.frame_format = frame_format;
			}

            sprite3.init(reg::load_sprite(params));
        }

        {
            reg::params::Sprite params;
			params.local.position = { (float)-500, (float)300 };
			params.local.scale = { (float)0.1, (float)0.1 };
			params.texture = "textures/tux.png";
			params.texture_index = (unsigned int)1;

            sprite4.init(reg::load_sprite(params));
        }

        {
            reg::params::Sprite params;
			params.local.position = { (float)-400, (float)300 };
			params.local.scale = { (float)0.1, (float)0.1 };
			params.texture = "textures/tux.png";

            sprite5.init(reg::load_sprite(params));
        }

        {
            reg::params::Sprite params;
			params.texture = "textures/einstein.png";
			params.modulation = rendering::ModulationRect{
				glm::vec4{ (float)1.0, (float)1.0, (float)0.2, (float)0.7 },
				glm::vec4{ (float)0.2, (float)1.0, (float)1.0, (float)0.7 },
				glm::vec4{ (float)1.0, (float)0.2, (float)1.0, (float)0.7 },
				glm::vec4{ (float)0.5, (float)0.5, (float)0.5, (float)0.7 }
			};

            sprite1.init(reg::load_sprite(params));
        }

        {
            reg::params::Sprite params;
			params.local.position = { (float)-300, (float)-200 };
			params.local.rotation = (float)0.3;
			params.local.scale = { (float)1.0, (float)1.0 };
			params.texture = "textures/godot.svg";
			params.svg_scale = (float)3.0;

            godot_icon.init(reg::load_sprite(params));
        }

        {
            reg::params::Sprite params;
			params.local.position = { (float)100, (float)-300 };
			params.local.scale = { (float)20, (float)20 };
			params.texture = "textures/knight.png";
			{
				reg::params::Sprite::AutoFrameFormat frame_format;
				params.frame_format = frame_format;
			}

            knight.init(reg::load_sprite(params));
        }

        {
            reg::params::PolyComposite params;
			params.local.position = { (float)-200, (float)200 };
			params.local.scale = { (float)60, (float)60 };
			{
				reg::params::PolyComposite::ConvexDecompositionMethod _method;
				_method.points.reserve(10);
				_method.points.push_back({ (float)-4, (float)0 });
				_method.points.push_back({ (float)-2, (float)-2 });
				_method.points.push_back({ (float)0, (float)-2 });
				_method.points.push_back({ (float)2, (float)-1 });
				_method.points.push_back({ (float)4, (float)1 });
				_method.points.push_back({ (float)2, (float)3 });
				_method.points.push_back({ (float)1, (float)3 });
				_method.points.push_back({ (float)-1, (float)0 });
				_method.points.push_back({ (float)-3, (float)1 });
				_method.points.push_back({ (float)-3, (float)2 });
				params.method = _method;
			}

            concave_shape.init(reg::load_poly_composite(std::move(params)));
        }

        {
            reg::params::NGon params;
			params.local.position = { (float)300, (float)200 };
			params.local.scale = { (float)200, (float)200 };
			params.bordered = true;
			params.ngon_base.points.reserve(8);
			params.ngon_base.points.push_back({ (float)1, (float)0 });
			params.ngon_base.points.push_back({ (float)0.707, (float)0.707 });
			params.ngon_base.points.push_back({ (float)0, (float)1 });
			params.ngon_base.points.push_back({ (float)-0.707, (float)0.707 });
			params.ngon_base.points.push_back({ (float)-1, (float)0 });
			params.ngon_base.points.push_back({ (float)-0.707, (float)-0.707 });
			params.ngon_base.points.push_back({ (float)0, (float)-1 });
			params.ngon_base.points.push_back({ (float)0.707, (float)-0.707 });
			params.ngon_base.fill_colors.reserve(1);
			params.ngon_base.fill_colors.push_back({ (float)0.0, (float)1.0, (float)0.0, (float)0.7 });
			params.ngon_base.border_colors.reserve(8);
			params.ngon_base.border_colors.push_back({ (float)0.25, (float)0.0, (float)0.5, (float)1.0 });
			params.ngon_base.border_colors.push_back({ (float)0.0, (float)0.25, (float)0.25, (float)1.0 });
			params.ngon_base.border_colors.push_back({ (float)0.25, (float)0.5, (float)0.0, (float)1.0 });
			params.ngon_base.border_colors.push_back({ (float)0.5, (float)0.75, (float)0.25, (float)1.0 });
			params.ngon_base.border_colors.push_back({ (float)0.75, (float)1.0, (float)0.5, (float)1.0 });
			params.ngon_base.border_colors.push_back({ (float)1.0, (float)0.75, (float)0.75, (float)1.0 });
			params.ngon_base.border_colors.push_back({ (float)0.75, (float)0.5, (float)1.0, (float)1.0 });
			params.ngon_base.border_colors.push_back({ (float)0.5, (float)0.25, (float)0.75, (float)1.0 });
			params.ngon_base.border_width = (float)0.05;

            octagon.init(reg::load_ngon(std::move(params)));
        }

        {
            reg::params::Paragraph params;
			params.local.position = { (float)-400, (float)400 };
			params.local.scale = { (float)0.8, (float)0.8 };
			params.font_atlas = "fonts/Roboto-Regular.ttf";
			params.text = "rgb x\txx  x.\nabcd !!!\r\n\n123478s\nHex";
			params.draw_bkg = true;
			params.bkg_color = { (float)0.5, (float)0.4, (float)0.2, (float)0.8 };
			params.text_color = { (float)0.0, (float)0.0, (float)0.0, (float)1.0 };
			params.glyph_colors.reserve(3);
			params.glyph_colors.push_back({ 0, { (float)1.0, (float)0.0, (float)0.0, (float)1.0 } });
			params.glyph_colors.push_back({ 1, { (float)0.0, (float)1.0, (float)0.0, (float)1.0 } });
			params.glyph_colors.push_back({ 2, { (float)0.0, (float)0.0, (float)1.0, (float)1.0 } });
			params.format.pivot = { (float)0.0, (float)1.0 };
			params.format.line_spacing = (float)1.5;
			params.format.linebreak_spacing = (float)2.0;
			params.format.min_size = { (float)800, (float)800 };
			params.format.padding = { (float)50, (float)50 };
			params.format.horizontal_alignment = rendering::ParagraphFormat::HorizontalAlignment::CENTER;
			params.format.vertical_alignment = rendering::ParagraphFormat::VerticalAlignment::MIDDLE;

            test_text.init(reg::load_paragraph(std::move(params)));
        }

        {
            reg::params::SpriteAtlas params;
			params.sprite_params.local.position = { (float)300, (float)-200 };
			params.sprite_params.local.scale = { (float)20, (float)20 };
			params.sprite_params.texture = "textures/knight.png";
			params.sprite_params.texture_index = (unsigned int)1;
			{
				reg::params::SpriteAtlas::Frame frame{ .rows = (GLuint)2, .cols = (GLuint)3, .delay_seconds = (float)0.1 };
				params.frame = frame;
			}

            atlased_knight.init(reg::load_sprite_atlas(params));
        }

        {
            reg::params::TileMap params;
			params.local.scale = { (float)100, (float)100 };
			{
				reg::params::TileMap::Layer _layer;
				_layer.tileset = "assets/grass tileset.toml";
				_layer.z = (size_t)0;
				_layer.tiles.reserve(13);
				_layer.tiles.push_back({ (float)-1, (float)-1 });
				_layer.tiles.push_back({ (float)0, (float)-1 });
				_layer.tiles.push_back({ (float)1, (float)-1 });
				_layer.tiles.push_back({ (float)-1, (float)0 });
				_layer.tiles.push_back({ (float)1, (float)0 });
				_layer.tiles.push_back({ (float)2, (float)0 });
				_layer.tiles.push_back({ (float)-1, (float)1 });
				_layer.tiles.push_back({ (float)0, (float)1 });
				_layer.tiles.push_back({ (float)1, (float)1 });
				_layer.tiles.push_back({ (float)2, (float)2 });
				_layer.tiles.push_back({ (float)3, (float)2 });
				_layer.tiles.push_back({ (float)2, (float)3 });
				_layer.tiles.push_back({ (float)3, (float)3 });
				params.layers.push_back(std::move(_layer));
			}

            grass_tilemap.init(reg::load_tilemap(params));
        }

        {
            reg::params::SpriteNonant params;
			params.sprite_params.local.scale = { (float)10.0, (float)10.0 };
			params.sprite_params.texture = "textures/panel.png";
			params.sprite_params.modulation = rendering::ModulationRect{
				glm::vec4{ (float)1.0, (float)0.3, (float)0.3, (float)1.0 },
				glm::vec4{ (float)0.3, (float)0.3, (float)0.3, (float)1.0 },
				glm::vec4{ (float)0.3, (float)0.3, (float)1.0, (float)1.0 },
				glm::vec4{ (float)0.3, (float)0.3, (float)0.3, (float)1.0 }
			};
			params.nsize = { (float)128, (float)0 };
			params.offsets.x_left = (float)6;
			params.offsets.x_right = (float)6;
			params.offsets.y_bottom = (float)6;
			params.offsets.y_top = (float)6;

            nonant_panel.init(reg::load_sprite_nonant(params));
        }

		sprite3->transformer.attach_parent(&transformer);
		sprite4->transformer.attach_parent(&transformer);
		sprite5->transformer.attach_parent(&transformer);
		sprite1->transformer.attach_parent(&transformer);
		godot_icon->transformer.attach_parent(&transformer);
		knight->transformer.attach_parent(&transformer);
		concave_shape->transformer.attach_parent(&transformer);
		octagon->transformer.attach_parent(&transformer);
		test_text->transformer.attach_parent(&transformer);
		atlased_knight->sprite.transformer.attach_parent(&transformer);
		grass_tilemap->set_transformer().attach_parent(&transformer);
		nonant_panel->set_transformer().attach_parent(&transformer);
    }


    void Jumble::draw(bool flush_sprites) const
    {
		sprite3->draw();
		sprite4->draw();
		sprite5->draw();
		context::render_sprites();
		octagon->draw();
		context::render_polygons();
		sprite1->draw();
		godot_icon->draw();
		knight->draw();
		context::render_sprites();
		concave_shape->draw();
		context::render_polygons();
		atlased_knight->draw();
		grass_tilemap->draw();
		context::render_sprites();
		test_text->draw();
		context::render_text();
		nonant_panel->draw();
		if (flush_sprites)
			context::render_sprites();
	}


    void Jumble::on_tick() const
    {
		atlased_knight->on_tick();
	}
}
