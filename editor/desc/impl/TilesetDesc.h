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

	struct TilesetAssignmentDesc
	{
		StringField texture;
		IntField<MakeOpt(0), MakeOpt<int>()> texture_index;
		UVRectField uvs;
		BitsetField<detail::TileReflection, detail::TILE_REFLECTION_BITSET_COUNT> reflection;
		EnumField<detail::TileRotation> rotation;

		DESCRIPTOR_BODY(TilesetAssignmentDesc, TILESET_ASSIGNMENT_GENERATOR);

		TilesetAssignmentDesc();
	};

#define TILESET_ASSIGNMENT_MAP_GENERATOR(M) \
		M(map)

	struct TilesetAssignmentMapDesc
	{
		MapDesc<detail::TileConfig, TilesetAssignmentDesc> map;

		DESCRIPTOR_BODY(TilesetAssignmentMapDesc, TILESET_ASSIGNMENT_MAP_GENERATOR);

		TilesetAssignmentMapDesc();
	};

#define TILESET_PARTIAL_GENERATOR(M) \
		M(storage)

#define TILESET_GENERATOR(M) \
		TILESET_PARTIAL_GENERATOR(M) \
		M(assignments)

	struct TilesetDesc
	{
		EnumField<detail::StorageMode> storage;
		TilesetAssignmentMapDesc assignments;
		static const detail::Key assignments_key;

		DESCRIPTOR_BODY(TilesetDesc, TILESET_GENERATOR);

		TilesetDesc();
	};
}
