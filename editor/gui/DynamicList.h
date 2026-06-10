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
			Move
		};

		Type type;
		bool valid = true;
		size_t index = 0;
		size_t src = 0;

		bool UpdateIndex(size_t& idx) const;

		static RowOperation MakeDelete(size_t index);
		static RowOperation MakeMove(size_t src, size_t dst);
	};

	struct DynamicListState
	{
		size_t list_size;
		size_t index = 0;
		std::vector<RowOperation> row_ops;
		std::unordered_set<size_t> simul_selected;

		void InitList(size_t count);
		void Clamp();
		void SetLast();

		void OnPushBack();
		void OnClear();
		void OnResize(size_t count);
		
		void DeferDelete();

		bool VisitRowOps(std::function<void(const RowOperation& op)> fn);
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
	};
}
