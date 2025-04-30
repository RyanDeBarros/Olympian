#include "SpriteRegistry.h"

#include "../../Olympian.h"

namespace oly
{
	namespace rendering
	{
		void SpriteRegistry::load(const Context& context, const char* sprite_registry_file)
		{
			auto toml = assets::load_toml(sprite_registry_file);
			auto toml_sprites = toml["sprite_registry"];
			if (!toml_sprites)
				return;
			auto sprite_list = toml_sprites["sprite"].as_array();
			if (!sprite_list)
				return;
			auto toml_texture_map = toml_sprites["texture_map"].as_table();
			if (!toml_texture_map)
				return;
			for (const auto& [k, v] : *toml_texture_map)
			{
				if (auto str = v.value<std::string>())
					texture_map[std::string(k.str())] = str.value();
			}
			sprite_list->for_each([this, &context](auto&& node) {
				if constexpr (toml::is_table<decltype(node)>)
				{
					if (auto _name = node["name"].value<std::string>())
					{
						const std::string& name = _name.value();
						sprite_constructors[name] = node;
						if (auto _init = node["init"].value<std::string>())
						{
							auto_loaded.emplace(std::move(name), create_sprite(context, name).share_moved());
							if (_init.value() == "discard")
								sprite_constructors.erase(name);
						}
					}
				}
				});

			auto extension_list = toml_sprites["extension"].as_array();
			if (extension_list)
			{
				extension_list->for_each([this](auto&& node) {
					if constexpr (toml::is_table<decltype(node)>)
					{
						auto _name = node["name"].value<std::string>();
						auto _type = node["extension"].value<std::string>();
						if (_name && _type)
						{
							const std::string& type = _type.value();
							if (type == "atlas")
							{
								const std::string& name = _name.value();
								atlas_constructors[name] = node;
								if (auto _init = node["init"].value<std::string>())
								{
									auto_loaded_atlas_extensions.emplace(std::move(name), std::shared_ptr<AtlasResExtension>(new AtlasResExtension(create_atlas_extension(name))));
									if (_init.value() == "discard")
										atlas_constructors.erase(name);
								}
							}
						}
					}
					});
			}
		}

		void SpriteRegistry::clear()
		{
			texture_map.clear();
			sprite_constructors.clear();
			auto_loaded.clear();
			atlas_constructors.clear();
			auto_loaded_atlas_extensions.clear();
		}

		Sprite SpriteRegistry::create_sprite(const Context& context, const std::string& name) const
		{
			auto it = sprite_constructors.find(name);
			if (it == sprite_constructors.end())
				throw Error(ErrorCode::UNREGISTERED_SPRITE);
			const auto& node = it->second;

			Sprite sprite = context.sprite();
			sprite.set_local() = assets::load_transform_2d(node, "transform");

			std::string texture;
			if (auto toml_texture = node["texture"].value<std::string>())
			{
				auto it = texture_map.find(toml_texture.value());
				if (it != texture_map.end())
					sprite.set_texture(context.texture_registry(), it->second);
				else
					sprite.set_texture(context.texture_registry(), toml_texture.value());
			}
			else if (auto toml_texture = node["texture"].value<int64_t>())
			{
				auto it = texture_map.find(std::to_string(toml_texture.value()));
				if (it != texture_map.end())
				{
					texture = it->second;
					sprite.set_texture(context.texture_registry(), texture);
				}
			}
			if (auto toml_modulation = node["modulation"].as_array())
			{
				if (toml_modulation->size() == 4)
				{
					if (toml_modulation->get(0)->is_array())
					{
						SpriteBatch::Modulation modulation;
						if (assets::parse_vec4(toml_modulation->get_as<toml::array>(0), modulation.colors[0])
							&& assets::parse_vec4(toml_modulation->get_as<toml::array>(1), modulation.colors[1])
							&& assets::parse_vec4(toml_modulation->get_as<toml::array>(2), modulation.colors[2])
							&& assets::parse_vec4(toml_modulation->get_as<toml::array>(3), modulation.colors[3]))
							sprite.set_modulation(modulation);
					}
					else
					{
						glm::vec4 modulation;
						if (assets::parse_vec4(toml_modulation, modulation))
							sprite.set_modulation({ modulation, modulation, modulation, modulation });
					}
				}
			}
			if (auto toml_tex_coords = node["tex coords"].as_array())
			{
				if (toml_tex_coords->size() == 4)
				{
					UVRect uvs;
					if (assets::parse_vec2(toml_tex_coords->get_as<toml::array>(0), uvs.uvs[0])
						&& assets::parse_vec2(toml_tex_coords->get_as<toml::array>(1), uvs.uvs[1])
						&& assets::parse_vec2(toml_tex_coords->get_as<toml::array>(2), uvs.uvs[2])
						&& assets::parse_vec2(toml_tex_coords->get_as<toml::array>(3), uvs.uvs[3]))
						sprite.set_tex_coords(uvs);
				}
			}
			if (auto toml_frame_format = node["frame_format"])
			{
				AnimFrameFormat frame_format;
				auto mode = toml_frame_format["mode"].value<std::string>();
				if (mode && mode == "single")
					frame_format = setup_anim_frame_format_single(context, texture, (GLuint)toml_frame_format["frame"].value<int64_t>().value_or(0));
				else if (mode && mode == "auto")
					frame_format = setup_anim_frame_format(context, texture, (float)toml_frame_format["speed"].value<double>().value_or(1.0),
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
						assets::parse_vec2(node["shearing"].as_array(), modifier.shearing);
					}
					else if (type == "pivot")
					{
						sprite.transformer.modifier = std::make_unique<PivotTransformModifier2D>();
						auto& modifier = sprite.transformer.get_modifier<PivotTransformModifier2D>();
						assets::parse_vec2(node["pivot"].as_array(), modifier.pivot);
						assets::parse_vec2(node["size"].as_array(), modifier.size);
					}
					else if (type == "pivot-shear")
					{
						sprite.transformer.modifier = std::make_unique<PivotShearTransformModifier2D>();
						auto& modifier = sprite.transformer.get_modifier<PivotShearTransformModifier2D>();
						assets::parse_vec2(node["shearing"].as_array(), modifier.shearing);
						assets::parse_vec2(node["pivot"].as_array(), modifier.pivot);
						assets::parse_vec2(node["size"].as_array(), modifier.size);
					}
				}
			}

			return sprite;
		}

		std::weak_ptr<Sprite> SpriteRegistry::ref_sprite(const std::string& name) const
		{
			auto it = auto_loaded.find(name);
			if (it == auto_loaded.end())
				throw Error(ErrorCode::UNREGISTERED_SPRITE);
			return it->second;
		}

		void SpriteRegistry::delete_sprite(const std::string& name)
		{
			auto_loaded.erase(name);
		}

		AtlasResExtension SpriteRegistry::create_atlas_extension(const std::string& name) const
		{
			auto it = atlas_constructors.find(name);
			if (it == atlas_constructors.end())
				throw Error(ErrorCode::UNREGISTERED_ATLAS);
			const auto& node = it->second;

			AtlasResExtension atlas;

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
		std::weak_ptr<AtlasResExtension> SpriteRegistry::ref_atlas_extension(const std::string& name) const
		{
			auto it = auto_loaded_atlas_extensions.find(name);
			if (it == auto_loaded_atlas_extensions.end())
				throw Error(ErrorCode::UNREGISTERED_ATLAS);
			return it->second;
		}

		void SpriteRegistry::delete_atlas_extension(const std::string& name)
		{
			auto_loaded_atlas_extensions.erase(name);
		}
	}
}
