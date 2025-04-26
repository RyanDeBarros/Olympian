#include "SpriteRegistry.h"

#include "../../Olympian.h"

namespace oly
{
	namespace rendering
	{
		void SpriteRegistry::load(const char* sprite_registry_file)
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
			sprite_list->for_each([this](auto&& node) {
				if constexpr (toml::is_table<decltype(node)>)
				{
					if (auto _name = node["name"].value<std::string>())
						sprite_constructors[_name.value()] = node;
				}
				});

			auto toml_draw_lists = toml_sprites["draw_list"].as_array();
			if (toml_draw_lists)
			{
				toml_draw_lists->for_each([this](auto&& node) {
					if constexpr (toml::is_table<decltype(node)>)
					{
						auto _name = node["name"].value<std::string>();
						if (!_name)
							return;

						DrawList draw_list;
						auto sprites = node["sprites"].as_array();
						if (sprites)
						{
							for (const auto& toml_sprite_name : *sprites)
							{
								auto sprite_name = toml_sprite_name.value<std::string>();
								if (sprite_name)
									draw_list.push_back(std::move(sprite_name.value()));
							}
						}
						if (!draw_list.empty())
							draw_lists.emplace(_name.value(), std::move(draw_list));
					}
					});
			}
		}

		void SpriteRegistry::clear()
		{
			texture_map.clear();
			sprite_constructors.clear();
			registered_sprites.clear();
			draw_lists.clear();
		}

		Sprite SpriteRegistry::create_sprite(const Context* context, const std::string& name) const
		{
			auto it = sprite_constructors.find(name);
			if (it == sprite_constructors.end())
				throw Error(ErrorCode::UNREGISTERED_SPRITE);
			const auto& node = it->second;

			Sprite sprite = context->sprite();
			sprite.local() = assets::load_transform_2d(node, "transform");
			sprite.post_set();

			std::string texture;
			if (auto toml_texture = node["texture"].value<std::string>())
			{
				auto it = texture_map.find(toml_texture.value());
				if (it != texture_map.end())
					sprite.set_texture(&context->texture_registry(), it->second);
				else
					sprite.set_texture(&context->texture_registry(), toml_texture.value());
			}
			else if (auto toml_texture = node["texture"].value<int64_t>())
			{
				auto it = texture_map.find(std::to_string(toml_texture.value()));
				if (it != texture_map.end())
				{
					texture = it->second;
					sprite.set_texture(&context->texture_registry(), texture);
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
					SpriteBatch::TexUVRect uvs;
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

		void SpriteRegistry::register_sprite(const Context* context, const std::string& name) const
		{
			if (!registered_sprites.count(name))
				registered_sprites.emplace(name, std::shared_ptr<Sprite>(new Sprite(create_sprite(context, name))));
		}

		std::weak_ptr<Sprite> SpriteRegistry::get_sprite(const Context* context, const std::string& name, bool register_if_nonexistant) const
		{
			auto it = registered_sprites.find(name);
			if (it != registered_sprites.end())
				return it->second;
			if (register_if_nonexistant)
			{
				register_sprite(context, name);
				return registered_sprites.find(name)->second;
			}
			else
				return {};
		}

		void SpriteRegistry::delete_sprite(const Context* context, const std::string& name) const
		{
			registered_sprites.erase(name);
		}

		void SpriteRegistry::draw_sprites(const Context* context, const std::string& draw_list_name, bool register_if_nonexistant) const
		{
			auto it = draw_lists.find(draw_list_name);
			if (it != draw_lists.end())
			{
				const auto& draw_list = it->second;
				for (const auto& sprite : draw_list)
				{
					auto ptr = get_sprite(context, sprite, register_if_nonexistant);
					if (auto sp = ptr.lock())
						sp->draw();
				}
			}
			else
				throw Error(ErrorCode::UNREGISTERED_DRAW_LIST);
		}
	}
}
