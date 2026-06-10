#include "DynamicList.h"

#include "core/UID.h"
#include "gui/Toolbar.h"

namespace oly::editor::gui
{
	struct DynamicListStatePayload
	{
		const DynamicListState* identity;
		size_t index;
	};

	void DynamicListState::Clamp(size_t count)
	{
		if (index >= count)
			SetLast(count);
	}

	void DynamicListState::SetLast(size_t count)
	{
		index = count > 0 ? count - 1 : 0;
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
					if (src->identity == &_state)
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
