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
		M(transformations)

	struct TilesetAssignmentDesc
	{
		StringField texture;
		IntField<MakeOpt(0), MakeOpt<int>()> texture_index;
		UVRectField uvs;
		BitsetField<detail::TileTransformation, detail::TILE_TRANSFORMATION_COUNT> transformations;

		TilesetAssignmentDesc();
	};

	struct TilesetAssignmentMapDesc
	{
		MapDesc<detail::TileConfig, TilesetAssignmentDesc> map;

		TilesetAssignmentMapDesc();
	};

#define TILESET_PARTIAL_GENERATOR(M) \
		M(storage)

	struct TilesetDesc
	{
		EnumField<detail::StorageMode> storage;
		TilesetAssignmentMapDesc assignments;
		static const detail::Key assignments_key;

		TilesetDesc();
	};
}
