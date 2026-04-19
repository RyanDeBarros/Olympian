#include "Tilesets.h"

#include "graphics/sprites/TileSet.h"

#include "core/util/LoggerOperators.h"
#include "core/util/Loader.h"
#include "core/util/Parser.h"
#include "core/util/MetaSplitter.h"
#include "core/base/Definitions.h"

#include ".gen/keys/General.inl"

#include ".gen/enums/StorageMode.inl"

namespace oly::context
{
	namespace internal
	{
		std::unordered_map<ResourcePath, rendering::TileSetRef> tilesets;
	}

	struct TerminateTilesets
	{
		void operator()() const
		{
			internal::tilesets.clear();
		}
	};

	rendering::TileSetRef load_tileset(const ResourcePath& file)
	{
		SingletonTickService<TickPhase::None, void, TerminatePhase::Graphics, TerminateTilesets>::instance();

		if (file.empty())
		{
			_OLY_ENGINE_LOG_ERROR("CONTEXT") << "Filename is empty" << LOG.nl;
			throw Error(ErrorCode::LoadAsset);
		}

		auto it = internal::tilesets.find(file);
		if (it != internal::tilesets.end())
			return it->second;

		_OLY_ENGINE_LOG_DEBUG("CONTEXT") << "Parsing tileset [" << file << "]..." << LOG.nl;

		if (!io::MetaSplitter::meta(file).has_type("tileset"))
		{
			_OLY_ENGINE_LOG_ERROR("CONTEXT") << "Meta fields do not contain tileset type" << LOG.nl;
			throw Error(ErrorCode::LoadAsset);
		}

		auto toml = io::load_toml(file);
		auto tileset = rendering::TileSetRef::load((TOMLNode)toml);

		if (assets::Parser(toml).translate<_gen::StorageMode>().defaulted(_gen::keys::General::Storage)(StorageMode::Discard) == StorageMode::Keep)
			internal::tilesets.emplace(file, tileset);

		_OLY_ENGINE_LOG_DEBUG("CONTEXT") << "...Tileset [" << file << "] parsed" << LOG.nl;

		return tileset;
	}

	void free_tileset(const ResourcePath& file)
	{
		internal::tilesets.erase(file);
	}
}
