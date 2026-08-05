#include "DynamicList.h"

#include "core/editor/ResourceLoader.h"

#include "gui/graphics/Toolbar.h"

#include <imtk.hpp>

namespace oly::editor::gui
{
	bool RowOperation::UpdateIndex(size_t& idx) const
	{
		switch (type)
		{
		case Type::Delete:
			if (idx == GetIndex())
				return false;

			if (idx > GetIndex())
				--idx;

			break;

		case Type::Move:
		{
			size_t min = std::min(GetSrcIndex(), GetDstIndex());
			size_t max = std::max(GetSrcIndex(), GetDstIndex());

			if (idx >= min && idx <= max)
			{
				if (idx == GetSrcIndex())
					idx = GetDstIndex();
				else if (GetSrcIndex() < GetDstIndex())
					--idx;
				else
					++idx;
			}

			break;
		}

		case Type::Resize:
			if (idx >= GetSize())
				return false;

			break;

		case Type::PushBack:
			break;
		}

		return true;
	}

	void RowOperation::UpdateRowOp(RowOperation& op) const
	{
		op.SetValid(op.valid && UpdateIndex(op.index1) && UpdateIndex(op.index2));
	}

	RowOperation RowOperation::MakeDelete(size_t index)
	{
		return RowOperation{ .type = Type::Delete, .index1 = index, .index2 = index };
	}

	RowOperation RowOperation::MakeMove(size_t src, size_t dst)
	{
		return RowOperation{ .type = Type::Move, .index1 = src, .index2 = dst };
	}

	RowOperation RowOperation::MakeResize(size_t size)
	{
		return RowOperation{ .type = Type::Resize, .index1 = size, .index2 = size };
	}

	RowOperation RowOperation::MakePushBack()
	{
		return RowOperation{ .type = Type::PushBack, .index1 = 0, .index2 = 0 };
	}

	size_t RowOperation::GetIndex() const
	{
		return index1;
	}

	size_t RowOperation::GetSrcIndex() const
	{
		return index1;
	}

	size_t RowOperation::GetDstIndex() const
	{
		return index2;
	}

	size_t RowOperation::GetSize() const
	{
		return index2;
	}

	void RowOperation::SetValid(bool valid)
	{
		if (type != Type::Resize && type != Type::PushBack)
			this->valid = valid;
	}

	struct DynamicListStatePayload : public imtk::drag_droppable_pod<DynamicListStatePayload>
	{
		const DynamicListState* identity;
		size_t index;

		DynamicListStatePayload(const DynamicListState* identity, size_t index)
			: identity(identity), index(index)
		{
		}
	};
}

namespace oly::editor::gui
{
	void DynamicListState::Clamp()
	{
		if (index >= list_size)
			SetLast();
	}

	void DynamicListState::SetLast()
	{
		index = list_size > 0 ? list_size - 1 : 0;
	}

	void DynamicListState::DeferPushBack()
	{
		row_ops.push_back(RowOperation::MakePushBack());
	}

	void DynamicListState::DeferDelete()
	{
		if (!simul_selected.count(index))
		{
			if (index < list_size)
				row_ops.push_back(RowOperation::MakeDelete(index));
		}

		for (size_t idx : simul_selected_ordered)
		{
			if (idx < list_size)
				row_ops.push_back(RowOperation::MakeDelete(idx));
		}
	}

	void DynamicListState::DeferResize(size_t count)
	{
		row_ops.push_back(RowOperation::MakeResize(count));
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

			switch (it->type)
			{
			case RowOperation::Type::Delete:
				--list_size;
				break;
				
			case RowOperation::Type::Move:
				break;

			case RowOperation::Type::Resize:
				list_size = it->GetSize();
				break;

			case RowOperation::Type::PushBack:
				++list_size;
				SetLast();
				break;
			}

			if (!it->UpdateIndex(index))
				Clamp();

			std::unordered_set<size_t> keep_selected;
			for (auto ut = simul_selected_ordered.begin(); ut != simul_selected_ordered.end(); )
			{
				if (it->UpdateIndex(*ut))
					keep_selected.insert(*ut++);
				else
					ut = simul_selected_ordered.erase(ut);
			}
			simul_selected = std::move(keep_selected);

			for (auto ut = std::next(it); ut != row_ops.end(); ++ut)
				it->UpdateRowOp(*ut);
		}

		row_ops.clear();
		return any;
	}

	void DynamicListState::DrawListHeader(size_t count)
	{
		list_size = count;
		Clamp();

		if (Toolbar::DrawIconButton(IconResource::Plus, "New item", "##Add"))
			DeferPushBack();

		if (auto d = imtk::disabled(list_size == 0))
		{
			ImGui::SameLine();
			if (Toolbar::DrawIconButton(IconResource::Minus, "Remove item (Del)", "##Remove"))
				DeferDelete();

			ImGui::SameLine();
			if (Toolbar::DrawIconButton(IconResource::Close, "Clear items", "##Clear"))
				DeferResize(0);
		}
	}

	void DynamicListState::DrawBody(std::function<void(DynamicRow&)> row_draw)
	{
		if (auto _ = imtk::child("List"))
		{
			for (size_t i = 0; i < list_size; ++i)
			{
				imtk::id_scope scope(i);

				if (auto row = gui::DynamicRow(i, "Row", *this))
					row_draw(row);
			}
		}

		if (ImGui::IsWindowFocused(ImGuiFocusedFlags_ChildWindows) && !ImGui::GetIO().WantTextInput && ImGui::Shortcut(ImGuiKey_Delete))
			DeferDelete();
	}

	DynamicRow::DynamicRow(size_t index, const char* str_id, DynamicListState& state)
		: _state(state), _index(index)
	{
		_cursor = ImGui::GetCursorScreenPos();
		_size = ImVec2(ImGui::GetContentRegionAvail().x, ImGui::GetFrameHeight());
		_child = std::make_unique<imtk::child>(str_id, _size, ImGuiChildFlags_AutoResizeX);

		if (*_child)
		{
			_visible = true;

			if (Toolbar::DrawHandle("##Drag"))
				OnSelect();

			if (auto _ = imtk::drag_drop_source())
			{
				OnSelect();

				imtk::send_drag_drop_payload(DynamicListStatePayload(&_state, _index));
				ImGui::TextUnformatted("Move row");
			}

			if (auto target = imtk::drag_drop_target())
			{
				if (auto payload = target.accept<DynamicListStatePayload>())
				{
					if ((*payload)->identity == &_state && (*payload)->index != _index)
						_state.row_ops.push_back(RowOperation::MakeMove((*payload)->index, _index));
				}
			}
		}
	}

	DynamicRow::~DynamicRow()
	{
		_child.reset();
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
			{
				if (!_state.simul_selected.contains(_state.index))
				{
					_state.simul_selected.insert(_state.index);
					_state.simul_selected_ordered.push_back(_state.index);
				}

				_state.index = _index;
			}
			else
			{
				if (_state.simul_selected.contains(_index))
				{
					_state.simul_selected.erase(_index);
					_state.simul_selected_ordered.erase(std::find(_state.simul_selected_ordered.begin(), _state.simul_selected_ordered.end(), _index));
				}
				
				if (!_state.simul_selected.empty())
				{
					_state.index = _state.simul_selected_ordered.back();
					_state.simul_selected_ordered.pop_back();
					_state.simul_selected.erase(_state.index);
				}
			}
		}
		else if (ImGui::GetIO().KeyShift)
		{
			size_t min = std::min(_index, _state.index);
			size_t max = std::max(_index, _state.index);

			for (size_t i = min; i <= max; ++i)
			{
				if (i != _index)
				{
					if (!_state.simul_selected.contains(i))
					{
						_state.simul_selected.insert(i);
						_state.simul_selected_ordered.push_back(i);
					}
				}
			}

			_state.index = _index;
		}
		else
		{
			_state.simul_selected.clear();
			_state.simul_selected_ordered.clear();
			_state.index = _index;
		}
	}

	size_t DynamicRow::Index() const
	{
		return _index;
	}
}
