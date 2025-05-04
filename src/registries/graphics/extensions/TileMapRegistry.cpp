#include "TileMapRegistry.h"

#include "registries/Loader.h"

namespace oly::reg
{
	void TileMapRegistry::load(const char* tilemap_file)
	{
		auto toml = load_toml(tilemap_file);
		auto tilemap_list = toml["tilemap"].as_array();
		if (!tilemap_list)
			return;
		tilemap_list->for_each([this](auto&& node) {
			if constexpr (toml::is_table<decltype(node)>)
			{
				if (auto _name = node["name"].value<std::string>())
				{
					const std::string& name = _name.value();
					tilemap_constructors[name] = node;
					if (auto _init = node["init"].value<std::string>())
					{
						auto_loaded.emplace(name, move_shared(create_tilemap(name)));
						if (_init.value() == "discard")
							tilemap_constructors.erase(name);
					}
				}
			}
			});
	}

	void TileMapRegistry::clear()
	{
		tilemap_constructors.clear();
		auto_loaded.clear();
	}

	rendering::TileMap TileMapRegistry::create_tilemap(const std::string& name) const
	{
		auto it = tilemap_constructors.find(name);
		if (it == tilemap_constructors.end())
			throw Error(ErrorCode::UNREGISTERED_TILEMAP);
		const auto& node = it->second;

		rendering::TileMap tilemap;
		tilemap.load(node);
		return tilemap;
	}

	std::weak_ptr<rendering::TileMap> TileMapRegistry::ref_tilemap(const std::string& name) const
	{
		auto it = auto_loaded.find(name);
		if (it == auto_loaded.end())
			throw Error(ErrorCode::UNREGISTERED_TILEMAP);
		return it->second;
	}

	void TileMapRegistry::delete_tilemap(const std::string& name)
	{
		auto_loaded.erase(name);
	}
}
