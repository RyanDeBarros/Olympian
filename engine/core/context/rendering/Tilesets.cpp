#include "Tilesets.h"

#include "graphics/sprites/TileSet.h"

#include "core/util/LoggerOperators.h"
#include "core/util/Loader.h"
#include "core/util/Parser.h"

#include "assets/MetaSplitter.h"
#include "definitions/Keys.h"
#include "definitions/enums/StorageMode.h"

namespace oly::context
{
	namespace internal
	{
		std::unordered_map<detail::ResourcePath, rendering::TileSetRef> tilesets;
	}

	struct TerminateTilesets
	{
		void operator()() const
		{
			internal::tilesets.clear();
		}
	};

	rendering::TileSetRef load_tileset(const detail::ResourcePath& file)
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

		if (!detail::MetaSplitter::decode_meta(file).has_type(detail::Key::Meta_Tileset))
		{
			_OLY_ENGINE_LOG_ERROR("CONTEXT") << "Meta fields do not contain tileset type" << LOG.nl;
			throw Error(ErrorCode::LoadAsset);
		}

		auto toml = io::load_toml(file);
		toml.insert_or_assign(detail::encode_key(detail::Key::InjectedSourceFile), file.string());
		auto tileset = rendering::TileSetRef::load((TOMLNode)toml);

		if (assets::Parser(toml).defaulted(detail::Key::Storage)(detail::StorageMode::Keep) == detail::StorageMode::Keep)
			internal::tilesets.emplace(file, tileset);

		_OLY_ENGINE_LOG_DEBUG("CONTEXT") << "...Tileset [" << file << "] parsed" << LOG.nl;

		return tileset;
	}

	void free_tileset(const detail::ResourcePath& file)
	{
		internal::tilesets.erase(file);
	}
}
