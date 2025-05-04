#include "TileSetRegistry.h"

#include "registries/Loader.h"

namespace oly::reg
{
	void TileSetRegistry::load(const char* tileset_file)
	{
		auto toml = load_toml(tileset_file);
		auto tileset_list = toml["tileset"].as_array();
		if (!tileset_list)
			return;
		tileset_list->for_each([this](auto&& node) {
			if constexpr (toml::is_table<decltype(node)>)
			{
				if (auto _name = node["name"].value<std::string>())
				{
					auto assignments = node["assignment"].as_array();
					if (!assignments)
						return;
					const std::string& name = _name.value();
					tileset_constructors[name] = std::move(*assignments);
					if (auto _init = node["init"].value<std::string>())
					{
						auto_loaded.emplace(name, move_shared(create_tileset(name)));
						if (_init.value() == "discard")
							tileset_constructors.erase(name);
					}
				}
			}
			});
	}

	void TileSetRegistry::clear()
	{
		tileset_constructors.clear();
		auto_loaded.clear();
	}

	rendering::TileSet TileSetRegistry::create_tileset(const std::string& name) const
	{
		auto it = tileset_constructors.find(name);
		if (it == tileset_constructors.end())
			throw Error(ErrorCode::UNREGISTERED_TILESET);
		const auto& node = it->second;

		rendering::TileSet tileset;
		tileset.load(node);
		return tileset;
	}

	std::weak_ptr<rendering::TileSet> TileSetRegistry::ref_tileset(const std::string& name) const
	{
		auto it = auto_loaded.find(name);
		if (it == auto_loaded.end())
			throw Error(ErrorCode::UNREGISTERED_TILESET);
		return it->second;
	}

	void TileSetRegistry::delete_tileset(const std::string& name)
	{
		auto_loaded.erase(name);
	}
}
