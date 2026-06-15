#include "TilesetDesc.h"

#include "definitions/Keys.h"

namespace oly::editor
{
	TilesetAssignmentDesc::TilesetAssignmentDesc() :
		texture("", detail::Key::TextureFile, "Texture"),
		texture_index(0, detail::Key::TextureIndex, "Texture Slot"),
		config(detail::TileConfiguration::Single, detail::Key::Configuration, "Configuration"),
		uvs({ 0.f, 1.f, 0.f, 1.f }, detail::Key::UVvec4, "Texture UVs"),
		transformations({}, detail::Key::TransformationArray, "Transformations")
	{
	}

	const detail::Key TilesetDesc::assignments_key = detail::Key::AssignmentArray;

	TilesetDesc::TilesetDesc() :
		storage(detail::StorageMode::Discard, detail::Key::Storage, "Storage")
	{
	}
}
