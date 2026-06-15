#pragma once

#include "core/types/SmartReference.h"
#include "core/math/Shapes.h"

#include "assets/ResourcePath.h"
#include "definitions/enums/TilesetConfiguration.h"

#include <unordered_set>

namespace oly::rendering
{
	class TileSet
	{
		struct Tile
		{
			size_t tex_index;
			detail::TileTransformation transformation = detail::TileTransformation::None;
		};

	public:
		struct TileDesc
		{
			detail::ResourcePath file;
			math::UVRect uvs;
			unsigned int file_index = 0;

			bool operator==(const TileDesc&) const = default;
		};

		struct Assignment
		{
			TileDesc desc;
			detail::TileConfig config = 0;
			detail::TileTransformation transformation = detail::TileTransformation::None;
		};

	private:
		std::vector<TileDesc> tiles;
		std::unordered_map<detail::TileConfig, Tile> assignments;

	public:
		TileSet(const std::vector<Assignment>& assignments = {});

	private:
		void load_assignments(const std::vector<Assignment>& assignments);

	public:
		TileDesc get_tile_desc(const detail::TileConfigGrid tile, detail::TileTransformation& transformation) const;

	private:
		std::optional<Tile> get_assignment(detail::TileConfig config, detail::TileTransformation& transformation, std::unordered_set<detail::TileConfig>& fallbacks_seen) const;

	public:
		void overload(TOMLNode node);

		static TileSet load(TOMLNode node)
		{
			TileSet tileset;
			tileset.overload(node);
			return tileset;
		}
	};

	typedef SmartReference<TileSet> TileSetRef;
}
