#include "DynamicList.h"

#include "core/UID.h"
#include "gui/Toolbar.h"

namespace oly::editor::gui
{
	bool RowOperation::UpdateIndex(size_t& idx) const
	{
		switch (type)
		{
		case Type::Delete:
			if (idx == index)
				return false;

			if (idx > index)
				--idx;

			break;

		case Type::Move:
			size_t min = std::min(src, index);
			size_t max = std::max(src, index);

			if (idx >= min && idx <= max)
			{
				if (idx == src)
					idx = index;
				else if (src < index)
					--idx;
				else
					++idx;
			}

			break;
		}

		return true;
	}

	RowOperation RowOperation::MakeDelete(size_t index)
	{
		return RowOperation{ .type = Type::Delete, .index = index, .src = index };
	}
	
	RowOperation RowOperation::MakeMove(size_t src, size_t dst)
	{
		return RowOperation{ .type = Type::Move, .index = dst, .src = src };
	}

	struct DynamicListStatePayload
	{
		const DynamicListState* identity;
		size_t index;
	};

	void DynamicListState::InitList(size_t count)
	{
		list_size = count;
		Clamp();
	}

	void DynamicListState::Clamp()
	{
		if (index >= list_size)
			SetLast();
	}

	void DynamicListState::SetLast()
	{
		index = list_size > 0 ? list_size - 1 : 0;
	}

	void DynamicListState::OnPushBack()
	{
		++list_size;
		SetLast();
	}

	void DynamicListState::OnClear()
	{
		list_size = 0;
		index = 0;
	}

	void DynamicListState::OnResize(size_t count)
	{
		list_size = count;
		Clamp();
	}

	void DynamicListState::DeferDelete()
	{
		if (!simul_selected.count(index))
		{
			if (index < list_size)
				row_ops.push_back(RowOperation::MakeDelete(index));
		}

		for (size_t idx : simul_selected)
		{
			if (idx < list_size)
				row_ops.push_back(RowOperation::MakeDelete(idx));
		}
	}

	bool DynamicListState::VisitRowOps(std::function<void(const RowOperation& op)> fn)
	{
		bool any = false;

		for (auto it = row_ops.begin(); it != row_ops.end(); ++it)
		{
			if (!it->valid)
				continue;

			any = true;

			fn(*it);

			if (!it->UpdateIndex(index))
				Clamp();

			std::unordered_set<size_t> keep_selected;
			for (size_t idx : simul_selected)
			{
				if (it->UpdateIndex(idx))
					keep_selected.insert(idx);
			}
			simul_selected = std::move(keep_selected);

			for (auto ut = std::next(it); ut != row_ops.end(); ++ut)
				ut->valid = ut->valid && it->UpdateIndex(ut->index) && it->UpdateIndex(ut->src);
		}

		row_ops.clear();
		return any;
	}

	DynamicRow::DynamicRow(size_t index, const char* str_id, DynamicListState& state)
		: _state(state), _index(index)
	{
		_cursor = ImGui::GetCursorScreenPos();
		_size = ImVec2(ImGui::GetContentRegionAvail().x, ImGui::GetFrameHeight());

		if (ImGui::BeginChild(str_id, _size))
		{
			_visible = true;

			if (Toolbar::DrawHandle("##Drag"))
				OnSelect();

			if (ImGui::BeginDragDropSource())
			{
				DynamicListStatePayload payload{
					.identity = &_state,
					.index = _index
				};
				ImGui::SetDragDropPayload(StringID(UID::DynamicRowReorder), &payload, sizeof(DynamicListStatePayload));
				ImGui::Text("Move row");
				ImGui::EndDragDropSource();
			}

			if (ImGui::BeginDragDropTarget())
			{
				if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(StringID(UID::DynamicRowReorder)))
				{
					DynamicListStatePayload* src = reinterpret_cast<DynamicListStatePayload*>(payload->Data);
					if (src->identity == &_state && src->index != _index)
						_state.row_ops.push_back(RowOperation::MakeMove(src->index, _index));
				}

				ImGui::EndDragDropTarget();
			}
		}
	}

	DynamicRow::~DynamicRow()
	{
		ImGui::EndChild();

		if (ImGui::IsItemClicked())
			OnSelect();

		if (_state.index == _index)
			ImGui::GetWindowDrawList()->AddRectFilled(_cursor, _cursor + _size, ImGui::GetColorU32(ImGuiCol_FrameBgHovered));
		else if (_state.simul_selected.contains(_index))
			ImGui::GetWindowDrawList()->AddRectFilled(_cursor, _cursor + _size, ImGui::GetColorU32(ImGuiCol_FrameBgHovered, 0.5f));
	}

	DynamicRow::operator bool() const
	{
		return _visible;
	}

	void DynamicRow::OnSelect()
	{
		if (ImGui::GetIO().KeyCtrl)
		{
			if (_state.index != _index)
				_state.simul_selected.insert(_state.index);
		}
		else
			_state.simul_selected.clear();

		_state.index = _index;
	}
}
