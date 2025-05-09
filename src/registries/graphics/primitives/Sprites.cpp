#include "Sprites.h"

#include "core/base/Context.h"
#include "registries/Loader.h"

namespace oly::reg
{
	rendering::Sprite load_sprite(const TOMLNode& node)
	{
		rendering::Sprite sprite = context::sprite();
		sprite.set_local() = load_transform_2d(node, "transform");

		std::string texture;
		if (auto toml_texture = node["texture"].value<std::string>())
		{
			texture = toml_texture.value();
			unsigned int texture_index = (unsigned int)node["texture index"].value<int64_t>().value_or(0);
			if (auto sc = node["svg scale"].value<double>())
			{
				reg::TextureRegistry::SVGLoadParams params{ .texture_index = texture_index };
				auto btex = context::texture_registry().load_svg_texture(texture, (float)sc.value(), params);
				sprite.set_texture(btex, context::texture_registry().get_dimensions(texture, texture_index));
			}
			else
			{
				reg::TextureRegistry::LoadParams params{ .texture_index = texture_index };
				auto btex = context::texture_registry().load_texture(texture, params);
				sprite.set_texture(btex, context::texture_registry().get_dimensions(texture, texture_index));
			}
		}

		if (auto toml_modulation = node["modulation"].as_array())
		{
			if (toml_modulation->size() == 4)
			{
				if (toml_modulation->get(0)->is_array())
				{
					rendering::SpriteBatch::Modulation modulation;
					if (parse_vec4(toml_modulation->get_as<toml::array>(0), modulation.colors[0])
						&& parse_vec4(toml_modulation->get_as<toml::array>(1), modulation.colors[1])
						&& parse_vec4(toml_modulation->get_as<toml::array>(2), modulation.colors[2])
						&& parse_vec4(toml_modulation->get_as<toml::array>(3), modulation.colors[3]))
						sprite.set_modulation(modulation);
				}
				else
				{
					glm::vec4 modulation;
					if (parse_vec4(toml_modulation, modulation))
						sprite.set_modulation({ modulation, modulation, modulation, modulation });
				}
			}
		}

		if (auto toml_tex_coords = node["tex coords"].as_array())
		{
			if (toml_tex_coords->size() == 4)
			{
				rendering::UVRect uvs;
				if (parse_vec2(toml_tex_coords->get_as<toml::array>(0), uvs.uvs[0])
					&& parse_vec2(toml_tex_coords->get_as<toml::array>(1), uvs.uvs[1])
					&& parse_vec2(toml_tex_coords->get_as<toml::array>(2), uvs.uvs[2])
					&& parse_vec2(toml_tex_coords->get_as<toml::array>(3), uvs.uvs[3]))
					sprite.set_tex_coords(uvs);
			}
		}

		if (auto toml_frame_format = node["frame_format"])
		{
			graphics::AnimFrameFormat frame_format;
			auto mode = toml_frame_format["mode"].value<std::string>();
			if (!texture.empty() && mode && mode == "single")
				frame_format = graphics::setup_anim_frame_format_single(texture, (GLuint)toml_frame_format["frame"].value<int64_t>().value_or(0));
			else if (!texture.empty() && mode && mode == "auto")
				frame_format = graphics::setup_anim_frame_format(texture, (float)toml_frame_format["speed"].value<double>().value_or(1.0),
					(GLuint)toml_frame_format["starting frame"].value<int64_t>().value_or(0));
			else
			{
				frame_format.starting_frame = (GLuint)toml_frame_format["starting frame"].value<int64_t>().value_or(0);
				frame_format.num_frames = (GLuint)toml_frame_format["num frames"].value<int64_t>().value_or(0);
				frame_format.starting_time = (float)toml_frame_format["starting time"].value<double>().value_or(0.0);
				frame_format.delay_seconds = (float)toml_frame_format["delay seconds"].value<double>().value_or(0.0);
			}
			sprite.set_frame_format(frame_format);
		}

		if (auto toml_transformer_modifier = node["transform_modifier"])
		{
			auto toml_type = toml_transformer_modifier["type"].value<std::string>();
			if (toml_type)
			{
				std::string type = toml_type.value();
				if (type == "shear")
				{
					sprite.transformer.modifier = std::make_unique<ShearTransformModifier2D>();
					auto& modifier = sprite.transformer.get_modifier<ShearTransformModifier2D>();
					parse_vec2(node["shearing"].as_array(), modifier.shearing);
				}
				else if (type == "pivot")
				{
					sprite.transformer.modifier = std::make_unique<PivotTransformModifier2D>();
					auto& modifier = sprite.transformer.get_modifier<PivotTransformModifier2D>();
					parse_vec2(node["pivot"].as_array(), modifier.pivot);
					parse_vec2(node["size"].as_array(), modifier.size);
				}
				else if (type == "pivot-shear")
				{
					sprite.transformer.modifier = std::make_unique<PivotShearTransformModifier2D>();
					auto& modifier = sprite.transformer.get_modifier<PivotShearTransformModifier2D>();
					parse_vec2(node["shearing"].as_array(), modifier.shearing);
					parse_vec2(node["pivot"].as_array(), modifier.pivot);
					parse_vec2(node["size"].as_array(), modifier.size);
				}
			}
		}

		return sprite;
	}
}
