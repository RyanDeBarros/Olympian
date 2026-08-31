#include "ListModel.h"

#include "core/editor/ResourceLoader.h"

#include <string>

namespace oly::editor::gui
{
	void ListModel::Init(IListAdapter& adapter)
	{
		_size = adapter.Size();
		active_index = 0;
		EnforcePolicy(adapter);
	}

	void ListModel::Update(IListAdapter& adapter)
	{
		_size = adapter.Size();
		Clamp();
		EnforcePolicy(adapter);
	}

	size_t ListModel::Size() const
	{
		return _size;
	}

	void ListModel::Clamp()
	{
		if (active_index >= _size)
			SetLast();
	}
	
	void ListModel::SetLast()
	{
		active_index = _size > 0 ? _size - 1 : 0;
	}

	void ListModel::DeferCreate()
	{
		_pending_ops.push_back(imtk::list_op::make_append_op(_size));
	}
	
	void ListModel::DeferDelete()
	{
		_pending_ops.push_back(imtk::list_op::make_delete_op(active_index));
	}
	
	void ListModel::DeferResize(size_t new_size)
	{
		_pending_ops.push_back(imtk::list_op::make_resize_op(_size, new_size));
	}

	void ListModel::DeferClear()
	{
		_pending_ops.push_back(imtk::list_op::make_resize_op(_size, 0));
	}

	bool ListModel::ConsumeOps(IListAdapter& adapter)
	{
		bool any = false;

		for (auto it = _pending_ops.begin(); it != _pending_ops.end(); ++it)
		{
			if (!it->valid())
				continue;

			any = true;
			Apply(*it, adapter);

			for (auto ut = std::next(it); ut != _pending_ops.end(); )
			{
				it->update_op(policy, *ut);
				if (ut->valid())
					++ut;
				else
					ut = _pending_ops.erase(ut);
			}
		}

		_pending_ops.clear();
		return any;
	}

	void ListModel::Apply(const imtk::list_op& op, IListAdapter& adapter)
	{
		switch (op.type())
		{
		case imtk::list_op_type::append_:
			++_size;
			SetLast();
			break;

		case imtk::list_op_type::delete_:
			if (_size > 0)
				--_size;
			break;

		case imtk::list_op_type::resize_:
			_size = op.get_new_size();
			break;
		}

		adapter.Apply(op);

		if (!op.update_index(policy, active_index))
			Clamp();

		EnforcePolicy(adapter);
	}

	void ListModel::EnforcePolicy(IListAdapter& adapter)
	{
		if (_size == 0 && imp::has_flag(policy, imtk::list_policy::minimum_one))
			Apply(imtk::list_op::make_append_op(_size), adapter);
	}

	void ListModel::Invoke(const imtk::list_op& op, IListAdapter& adapter)
	{
		if (!_pending_ops.empty())
			ConsumeOps(adapter);

		Apply(op, adapter);
	}

	imtk::item_result ListModel::DrawComboHeader(const ComboHeader& header, const char* slot_prefix)
	{
		return DrawComboHeader(header, [slot_prefix](size_t i) { return slot_prefix + (" " + std::to_string(i)); });
	}

	imtk::item_result ListModel::DrawComboHeader(const ComboHeader& header, std::function<std::string(size_t)> combo_getter)
	{
		imtk::item_result result;

		imtk::style_color sc(ImGuiCol_ChildBg, ImGui::GetColorU32(ImGuiCol_FrameBg, 0.75f));

		if (auto _ = imtk::child(header.prompt, ImVec2(0, 0), ImGuiChildFlags_AutoResizeY | ImGuiChildFlags_Borders))
		{
			ImGui::TextUnformatted(header.prompt);
			ImGui::SameLine();

			imtk::item_result subresult;

			std::vector<std::string> slot_names;
			slot_names.reserve(_size);
			for (int i = 0; i < _size; ++i)
				slot_names.push_back(combo_getter(i));

			int slot = active_index;
			result |= imtk::controls::combo("##SelectSlot", slot, slot_names);
			active_index = slot;

			ImGui::SameLine();
			subresult = imtk::w::icon_button({ .icon = Icon(IconResource::Plus), .str_id = "##+", .tooltip = header.create_tooltip }).draw();
			result |= subresult;
			if (subresult.modified)
				DeferCreate();

			ImGui::SameLine();
			subresult = imtk::w::icon_button({ .icon = Icon(IconResource::Minus), .str_id = "##-", .tooltip = header.delete_tooltip }).draw();
			result |= subresult;
			if (subresult.modified)
				DeferDelete();

			ImGui::SameLine();
			subresult = imtk::w::icon_button({ .icon = Icon(IconResource::Close), .str_id = "##x", .tooltip = header.clear_tooltip }).draw();
			result |= subresult;
			if (subresult.modified)
				DeferClear();
		}

		return result;
	}
}
