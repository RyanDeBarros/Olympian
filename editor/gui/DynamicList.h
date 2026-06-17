#pragma once

#include <imgui.h>

#include <functional>
#include <optional>
#include <unordered_set>
#include <vector>

namespace oly::editor::gui
{
	struct RowOperation
	{
		enum class Type
		{
			Delete,
			Move,
			Resize,
			PushBack
		};

		Type type;
		bool valid = true;
		size_t index = 0;
		size_t src = 0;

		bool UpdateIndex(size_t& idx) const;

		static RowOperation MakeDelete(size_t index);
		static RowOperation MakeMove(size_t src, size_t dst);
		static RowOperation MakeResize(size_t size);
		static RowOperation MakePushBack();

		void SetValid(bool valid);
	};

	class DynamicRow;

	struct DynamicListState
	{
		size_t list_size = 0;
		size_t index = 0;
		std::vector<RowOperation> row_ops;
		std::unordered_set<size_t> simul_selected;
		std::vector<size_t> simul_selected_ordered;

		void Clamp();
		void SetLast();

		void DeferPushBack();
		void DeferDelete();
		void DeferResize(size_t count);

		bool VisitRowOps(std::function<void(const RowOperation& op)> fn);

		void DrawListHeader(size_t list_size);
		void DrawBody(std::function<void(DynamicRow& row)> row_draw);
	};

	class DynamicRow
	{
		bool _visible = false;
		DynamicListState& _state;
		ImVec2 _cursor, _size;
		size_t _index;

	public:
		DynamicRow(size_t index, const char* str_id, DynamicListState& state);
		DynamicRow(const DynamicRow&) = delete;
		DynamicRow(DynamicRow&&) = delete;
		~DynamicRow();

		operator bool() const;

		void OnSelect();
		size_t Index() const;
	};
}
