#include "SpriteRegistry.h"

#include "../../Olympian.h"

namespace oly
{
	namespace mut
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
					auto _name = node["name"].value<std::string>();
					if (_name)
					{
						std::string name = _name.value();
						sprite_constructors[name] = node;
					}
				}
				});
		}

		Sprite SpriteRegistry::create_sprite(const Context* context, const std::string& name) const
		{
			auto it = sprite_constructors.find(name);
			if (it == sprite_constructors.end())
				throw Error(ErrorCode::UNREGISTERED_SPRITE);
			const auto& node = it->second;

			Sprite sprite = context->mut.sprite();
			sprite.local() = assets::load_transform_2d((const assets::AssetNode&)node["transform"]);
			sprite.post_set();

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
					sprite.set_texture(&context->texture_registry(), it->second);
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
			if (auto toml_gif = node["gif"])
			{
				// TODO
			}
			// TODO transform modifier

			return sprite;
		}
	}
}
