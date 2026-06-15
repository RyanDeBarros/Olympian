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
		M(config) \
		M(uvs) \
		M(transformations)

	struct TilesetAssignmentDesc
	{
		StringField texture;
		IntField<MakeOpt(0), MakeOpt<int>()> texture_index;
		EnumField<detail::TileConfiguration> config;
		Vec4Field<MakeOpt(0.f), MakeOpt(1.f)> uvs; // TODO v8 draw with custom x1/y1/x2/y2 sublabels
		BitsetField<detail::TileTransformation, detail::TILE_TRANSFORMATION_COUNT> transformations;

		TilesetAssignmentDesc();
	};

#define TILESET_PARTIAL_GENERATOR(M) \
		M(storage)

	struct TilesetDesc
	{
		EnumField<detail::StorageMode> storage;
		VectorDesc<TilesetAssignmentDesc> assignments;
		static const detail::Key assignments_key;

		TilesetDesc();
	};
}
