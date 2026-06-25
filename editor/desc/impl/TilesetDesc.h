#pragma once

#include "desc/Fields.h"
#include "desc/Descriptors.h"

#include "definitions/enums/StorageMode.h"
#include "definitions/enums/TilesetConfiguration.h"

namespace oly::editor
{
#define TILESET_ASSIGNMENT_GENERATOR(M) \
		M(texture) \
		M(texture_index) \
		M(uvs) \
		M(reflection) \
		M(rotation)

#define TILESET_ASSIGNMENT_SUBPATH_GENERATOR(M) \
		TILESET_ASSIGNMENT_GENERATOR(M)

	struct TilesetAssignmentDesc
	{
		StringField texture;
		IntField<MakeOpt(0), MakeOpt<int>()> texture_index;
		UVRectField uvs;
		BitsetField<detail::TileReflection, detail::TILE_REFLECTION_BITSET_COUNT> reflection;
		EnumField<detail::TileRotation> rotation;

		GENERATE_SUBPATHS(TILESET_ASSIGNMENT_SUBPATH_GENERATOR);

		TilesetAssignmentDesc();
	};

#define TILESET_ASSIGNMENT_MAP_SUBPATH_GENERATOR(M) \
		M(map)

	struct TilesetAssignmentMapDesc
	{
		MapDesc<detail::TileConfig, TilesetAssignmentDesc> map;

		GENERATE_SUBPATHS(TILESET_ASSIGNMENT_MAP_SUBPATH_GENERATOR);

		TilesetAssignmentMapDesc();
	};

#define TILESET_PARTIAL_GENERATOR(M) \
		M(storage)

#define TILESET_SUBPATH_GENERATOR(M) \
		TILESET_PARTIAL_GENERATOR(M) \
		M(assignments)

	struct TilesetDesc
	{
		EnumField<detail::StorageMode> storage;
		TilesetAssignmentMapDesc assignments;
		static const detail::Key assignments_key;

		GENERATE_SUBPATHS(TILESET_SUBPATH_GENERATOR);

		TilesetDesc();
	};
}
