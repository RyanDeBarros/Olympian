#pragma once

#include <imtk.hpp>

#include <functional>
#include <optional>
#include <unordered_set>

// TODO v9.3 merge common 'operation' logic with ListModel

namespace oly::editor::gui
{
	class DynamicRow;

	struct DynamicListState
	{
		imtk::list_policy policy = imtk::list_policy::none;
		size_t list_size = 0;
		size_t index = 0;
		std::vector<imtk::list_op> row_ops;
		std::unordered_set<size_t> simul_selected;
		std::vector<size_t> simul_selected_ordered;

		void Clamp();
		void SetLast();

		void DeferPushBack();
		void DeferDelete();
		void DeferResize(size_t count);

		bool VisitRowOps(std::function<void(const imtk::list_op& op)> fn);

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
