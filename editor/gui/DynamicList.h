#pragma once

#include <imtk.hpp>

#include <functional>
#include <optional>
#include <unordered_set>

namespace oly::editor::gui
{
	class DynamicRow;

	// TODO v9.3 remove DynamicListState - just put DrawListHeader/DrawBody into widget draw logic
	struct DynamicListState
	{
		imtk::list_model model;

		void DrawListHeader(size_t list_size);
		void DrawBody(std::function<void(DynamicRow& row)> row_draw);
	};

	class DynamicRow
	{
		bool _visible = false;
		DynamicListState& _state;
		ImVec2 _cursor, _size;
		size_t _index;
		std::unique_ptr<imtk::child> _child;

	public:
		DynamicRow(size_t index, const char* str_id, DynamicListState& state);
		DynamicRow(const DynamicRow&) = delete;
		DynamicRow(DynamicRow&&) = delete;
		~DynamicRow();

		operator bool() const;

		void OnSelect();
		size_t Index() const;
		ImVec2 Size() const;
	};
}
