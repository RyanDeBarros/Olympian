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
	public:
		struct PaintedTile
		{
			bool orthogonal[4] = { false, false, false, false }; // right, top, left, bottom
			bool diagonal[4] = { false, false, false, false }; // top-right, top-left, bottom-left, bottom-right
		};

	private:
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
			detail::TileConfiguration config = detail::TileConfiguration::Single;
			detail::TileTransformation transformation = detail::TileTransformation::None;
		};

	private:
		std::vector<TileDesc> tiles;
		std::unordered_map<detail::TileConfiguration, Tile> assignment;

	public:
		TileSet(const std::vector<Assignment>& assignments = {});

	private:
		void load_assignments(const std::vector<Assignment>& assignments);
		bool valid_configuration(detail::TileConfiguration configuration) const;

	public:
		bool valid_6() const;
		bool valid_4x4() const;
		bool valid_4x4_2x2() const;
		TileDesc get_tile_desc(PaintedTile tile, detail::TileTransformation& transformation) const;

	private:
		Tile get_assignment(detail::TileConfiguration config, detail::TileTransformation& transformation) const;
		static detail::TileConfiguration get_configuration(PaintedTile tile);

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
