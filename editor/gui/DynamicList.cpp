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
		row_ops.push_back(imtk::list_op::make_append_op());
	}

	void DynamicListState::DeferDelete()
	{
		if (!simul_selected.count(index))
		{
			if (index < list_size)
				row_ops.push_back(imtk::list_op::make_delete_op(index));
		}

		for (size_t idx : simul_selected_ordered)
		{
			if (idx < list_size)
				row_ops.push_back(imtk::list_op::make_delete_op(idx));
		}
	}

	void DynamicListState::DeferResize(size_t count)
	{
		row_ops.push_back(imtk::list_op::make_resize_op(list_size, count));
	}

	bool DynamicListState::VisitRowOps(std::function<void(const imtk::list_op& op)> fn)
	{
		bool any = false;

		for (auto it = row_ops.begin(); it != row_ops.end(); ++it)
		{
			if (!it->valid())
				continue;

			any = true;

			fn(*it);

			switch (it->type())
			{
			case imtk::list_op_type::delete_:
				--list_size;
				break;
				
			case imtk::list_op_type::move_:
				break;

			case imtk::list_op_type::resize_:
				list_size = it->get_new_size();
				break;

			case imtk::list_op_type::append_:
				++list_size;
				SetLast();
				break;
			}

			if (!it->update_index(policy, index))
				Clamp();

			std::unordered_set<size_t> keep_selected;
			for (auto ut = simul_selected_ordered.begin(); ut != simul_selected_ordered.end(); )
			{
				if (it->update_index(policy, *ut))
					keep_selected.insert(*ut++);
				else
					ut = simul_selected_ordered.erase(ut);
			}
			simul_selected = std::move(keep_selected);

			for (auto ut = std::next(it); ut != row_ops.end(); ++ut)
				it->update_op(policy, *ut);
		}

		row_ops.clear();
		return any;
	}

	void DynamicListState::DrawListHeader(size_t count)
	{
		list_size = count;
		Clamp();

		if (imtk::w::icon_button({ .icon = Icon(IconResource::Plus), .str_id = "##Add", .tooltip = "New item" }).draw())
			DeferPushBack();

		if (auto d = imtk::disabled(list_size == 0))
		{
			ImGui::SameLine();
			if (imtk::w::icon_button({ .icon = Icon(IconResource::Minus), .str_id = "##Remove", .tooltip = "Remove item (Del)" }).draw())
				DeferDelete();

			ImGui::SameLine();
			if (imtk::w::icon_button({ .icon = Icon(IconResource::Close), .str_id = "##Clear", .tooltip = "Clear items" }).draw())
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
						_state.row_ops.push_back(imtk::list_op::make_move_op((*payload)->index, _index));
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
