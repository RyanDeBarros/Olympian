#include "DynamicList.h"

#include "core/editor/ResourceLoader.h"

#include <imtk.hpp>

namespace oly::editor::gui
{
	struct DynamicListStatePayload : public imtk::drag_droppable_pod<DynamicListStatePayload>
	{
		const DynamicListState* identity;
		size_t index;

		DynamicListStatePayload(const DynamicListState* identity, size_t index)
			: identity(identity), index(index)
		{
		}
	};

	void DynamicListState::DrawListHeader(size_t count)
	{
		model.sync(count);

		if (imtk::w::icon_button({ .icon = Icon(IconResource::Plus), .str_id = "##Add", .tooltip = "New item" }).draw())
			model.defer_append();

		if (auto d = imtk::disabled(model.size() == 0))
		{
			ImGui::SameLine();
			if (imtk::w::icon_button({ .icon = Icon(IconResource::Minus), .str_id = "##Remove", .tooltip = "Remove item (Del)" }).draw())
				model.defer_delete();

			ImGui::SameLine();
			if (imtk::w::icon_button({ .icon = Icon(IconResource::Close), .str_id = "##Clear", .tooltip = "Clear items" }).draw())
				model.defer_resize(0);
		}
	}

	void DynamicListState::DrawBody(std::function<void(DynamicRow&)> row_draw)
	{
		for (size_t i = 0; i < model.size(); ++i)
		{
			imtk::id_scope scope(i);

			if (auto row = gui::DynamicRow(i, "Row", *this))
				row_draw(row);
		}

		if (ImGui::IsWindowFocused(ImGuiFocusedFlags_ChildWindows) && !ImGui::GetIO().WantTextInput && ImGui::Shortcut(ImGuiKey_Delete))
			model.defer_delete();
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

			if (imtk::w::icon_button({ .icon = Icon(IconResource::Handle), .str_id = "##Drag", .tooltip = "Drag item"}).draw())
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
						_state.model.defer_move((*payload)->index, _index);
				}
			}
		}
	}

	DynamicRow::~DynamicRow()
	{
		_child.reset();
		if (ImGui::IsItemClicked())
			OnSelect();

		if (_state.model.index() == _index)
			ImGui::GetWindowDrawList()->AddRectFilled(_cursor, _cursor + _size, ImGui::GetColorU32(ImGuiCol_FrameBgHovered));
		else if (_state.model.index_selected(_index))
			ImGui::GetWindowDrawList()->AddRectFilled(_cursor, _cursor + _size, ImGui::GetColorU32(ImGuiCol_FrameBgHovered, 0.5f));
	}

	DynamicRow::operator bool() const
	{
		return _visible;
	}

	void DynamicRow::OnSelect()
	{
		_state.model.on_select(_index, ImGui::GetIO().KeyCtrl, ImGui::GetIO().KeyShift);
	}

	size_t DynamicRow::Index() const
	{
		return _index;
	}
}
