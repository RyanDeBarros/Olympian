#include "SpriteRegistry.h"

#include "core/base/Context.h"
#include "registries/Loader.h"

namespace oly::reg
{
	void SpriteRegistry::load(const char* sprite_registry_file)
	{
		auto toml = load_toml(sprite_registry_file);
		auto sprite_list = toml["sprite"].as_array();
		if (sprite_list)
		{
			sprite_list->for_each([this](auto&& node) {
				if constexpr (toml::is_table<decltype(node)>)
				{
					if (auto _name = node["name"].value<std::string>())
					{
						const std::string& name = _name.value();
						sprite_constructors[name] = node;
						if (auto _init = node["init"].value<std::string>())
						{
							auto_loaded_sprites.emplace(std::move(name), move_shared(create_sprite(name)));
							if (_init.value() == "discard")
								sprite_constructors.erase(name);
						}
					}
				}
				});
		}

		auto sprite_atlas_list = toml["sprite_atlas"].as_array();
		if (sprite_atlas_list)
		{
			sprite_atlas_list->for_each([this](auto&& node) {
				if constexpr (toml::is_table<decltype(node)>)
				{
					if (auto _name = node["name"].value<std::string>())
					{
						const std::string& name = _name.value();
						sprite_atlas_constructors[name] = node;
						if (auto _init = node["init"].value<std::string>())
						{
							auto_loaded_sprite_atlases.emplace(std::move(name), move_shared(create_atlas_extension(name)));
							if (_init.value() == "discard")
								sprite_atlas_constructors.erase(name);
						}
					}
				}
				});
		}
	}

	void SpriteRegistry::clear()
	{
		sprite_constructors.clear();
		auto_loaded_sprites.clear();
		sprite_atlas_constructors.clear();
		auto_loaded_sprite_atlases.clear();
	}

	rendering::Sprite SpriteRegistry::create_sprite(const std::string& name) const
	{
		auto it = sprite_constructors.find(name);
		if (it == sprite_constructors.end())
			throw Error(ErrorCode::UNREGISTERED_SPRITE);
		const auto& node = it->second;

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
				frame_format.num_frames     = (GLuint)toml_frame_format["num frames"].value<int64_t>().value_or(0);
				frame_format.starting_time  = (float)toml_frame_format["starting time"].value<double>().value_or(0.0);
				frame_format.delay_seconds  = (float)toml_frame_format["delay seconds"].value<double>().value_or(0.0);
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

	std::weak_ptr<rendering::Sprite> SpriteRegistry::ref_sprite(const std::string& name) const
	{
		auto it = auto_loaded_sprites.find(name);
		if (it == auto_loaded_sprites.end())
			throw Error(ErrorCode::UNREGISTERED_SPRITE);
		return it->second;
	}

	void SpriteRegistry::delete_sprite(const std::string& name)
	{
		auto_loaded_sprites.erase(name);
	}

	rendering::SpriteAtlasResExtension SpriteRegistry::create_atlas_extension(const std::string& name) const
	{
		auto it = sprite_atlas_constructors.find(name);
		if (it == sprite_atlas_constructors.end())
			throw Error(ErrorCode::UNREGISTERED_ATLAS);
		const auto& node = it->second;

		rendering::SpriteAtlasResExtension atlas;

		if (auto _sprite_name = node["sprite"].value<std::string>())
			atlas.sprite = ref_sprite(_sprite_name.value()).lock();

		auto _rows           = node["rows"].value<int64_t>();
		auto _cols           = node["cols"].value<int64_t>();
		auto _delay_seconds  = node["delay seconds"].value<double>();

		if (_rows && _cols && _delay_seconds)
		{
			auto row_major = node["row major"].value<bool>().value_or(true);
			auto row_up = node["row up"].value<bool>().value_or(true);
			atlas.setup_uniform((GLuint)_rows.value(), (GLuint)_cols.value(), (float)_delay_seconds.value(), row_major, row_up);

			if (auto _starting_frame = node["starting frame"].value<int64_t>())
				atlas.anim_format.starting_frame = (GLuint)_starting_frame.value();
			if (auto _starting_time = node["starting time"].value<double>())
				atlas.anim_format.starting_time = (float)_starting_time.value();
			if (auto _static_frame = node["static frame"].value<int64_t>())
				atlas.select_static_frame((GLuint)_static_frame.value());
		}

		return atlas;
	}
	std::weak_ptr<rendering::SpriteAtlasResExtension> SpriteRegistry::ref_atlas_extension(const std::string& name) const
	{
		auto it = auto_loaded_sprite_atlases.find(name);
		if (it == auto_loaded_sprite_atlases.end())
			throw Error(ErrorCode::UNREGISTERED_ATLAS);
		return it->second;
	}

	void SpriteRegistry::delete_atlas_extension(const std::string& name)
	{
		auto_loaded_sprite_atlases.erase(name);
	}
}
