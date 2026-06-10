#include "DynamicList.h"

#include "gui/Toolbar.h"

// TODO v8 remove
#include <iostream>

namespace oly::editor::gui
{
	DynamicListState::DynamicListState()
		: self(this)
	{
	}

	DynamicListState::DynamicListState(const DynamicListState& o)
		: self(this), index(o.index)
	{
	}
	
	DynamicListState::DynamicListState(DynamicListState&& o) noexcept
		: self(this), index(o.index)
	{
	}
	
	DynamicListState& DynamicListState::operator=(const DynamicListState& o)
	{
		if (this != &o)
		{
			index = o.index;
		}
		return *this;
	}
	
	DynamicListState& DynamicListState::operator=(DynamicListState&& o) noexcept
	{
		if (this != &o)
		{
			index = o.index;
		}
		return *this;
	}

	void DynamicListState::Clamp(size_t count)
	{
		if (index >= count)
			SetLast(count);
	}

	void DynamicListState::SetLast(size_t count)
	{
		index = count > 0 ? count - 1 : 0;
	}

	void DynamicListState::SendPayload(const char* type)
	{
		ImGui::SetDragDropPayload(type, this, sizeof(DynamicListState));
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
				_state.index = _index;

			// TODO v8 put in central database
			static const char* DYNAMIC_STRING_LIST_REORDER = "DYNAMIC_STRING_LIST_REORDER";

			if (ImGui::BeginDragDropSource())
			{
				_state.index = _index;
				_state.SendPayload(DYNAMIC_STRING_LIST_REORDER);
				ImGui::Text("Move item");
				ImGui::EndDragDropSource();

				std::cout << _index << std::endl;
			}

			if (ImGui::BeginDragDropTarget())
			{
				if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(DYNAMIC_STRING_LIST_REORDER))
				{
					gui::DynamicListState* src = reinterpret_cast<gui::DynamicListState*>(payload->Data);
					if (src->self == _state.self)
						_dropped_src = src->index;
				}

				ImGui::EndDragDropTarget();
			}

			ImGui::SameLine();
		}
	}

	DynamicRow::~DynamicRow()
	{
		ImGui::EndChild();

		if (ImGui::IsItemClicked())
			_state.index = _index;

		if (_state.index == _index)
			ImGui::GetWindowDrawList()->AddRectFilled(_cursor, _cursor + _size, ImGui::GetColorU32(ImGuiCol_FrameBgHovered));
	}

	DynamicRow::operator bool() const
	{
		return _visible;
	}

	std::optional<size_t> DynamicRow::GetDroppedSource() const
	{
		return _dropped_src;
	}
}
