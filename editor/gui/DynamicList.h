#pragma once

#include <imtk.hpp>

#include <functional>
#include <optional>
#include <unordered_set>

namespace oly::editor::gui
{
	struct DynamicListHeader : public imtk::w::dynamic_list_header
	{
		using imtk::w::dynamic_list_header::dynamic_list_header;

		DynamicListHeader(imtk::list_model& model);
	};

	// TODO v9.3 remove DynamicList - just put DrawListHeader/DrawBody into widget draw logic
	struct DynamicList
	{
		imtk::list_model model;
		DynamicListHeader header;
		imtk::w::dynamic_list_body body;

		DynamicList();
		DynamicList(const DynamicList& o);
		DynamicList(DynamicList&& o) noexcept;

		DynamicList& operator=(const DynamicList&) = default;
		DynamicList& operator=(DynamicList&&) noexcept = default;

		imtk::item_result Draw(size_t list_size);
	};
}
