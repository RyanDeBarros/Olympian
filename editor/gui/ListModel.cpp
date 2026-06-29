#include "ListModel.h"

#include "core/editor/ResourceLoader.h"

#include "gui/ImGuiWrapper.h"
#include "gui/graphics/Toolbar.h"

#include <string>

namespace oly::editor::gui
{
	ListOp ListOp::MakeCreateOp()
	{
		return ListOp{ .type = ListOpType::Create };
	}
	
	ListOp ListOp::MakeDeleteOp(size_t index)
	{
		return ListOp{ .type = ListOpType::Delete, .index1 = index };
	}
	
	ListOp ListOp::MakeClearOp()
	{
		return ListOp{ .type = ListOpType::Clear };
	}
	
	ListOp ListOp::MakeResizeOp(size_t old_size, size_t new_size)
	{
		return ListOp{ .type = ListOpType::Resize, .index1 = old_size, .index2 = new_size };
	}
	
	ListOp ListOp::MakeMoveOp(size_t src, size_t dst)
	{
		return ListOp{ .type = ListOpType::Move, .index1 = src, .index2 = dst };
	}

	size_t ListOp::GetIndex() const
	{
		return index1;
	}

	size_t ListOp::GetSrcIndex() const
	{
		return index1;
	}
	
	size_t ListOp::GetDstIndex() const
	{
		return index2;
	}

	size_t ListOp::GetOldSize() const
	{
		return index1;
	}

	size_t ListOp::GetNewSize() const
	{
		return index2;
	}

	void ListOp::Validate(bool valid)
	{
		if (type != ListOpType::Create && type != ListOpType::Clear && type != ListOpType::Resize)
			this->valid = valid;
	}

	bool ListOp::UpdateIndex(ListPolicy policy, size_t& idx) const
	{
		Modifiable<size_t> m(idx);
		bool v = UpdateIndex(policy, m);
		idx = m;
		return v;
	}

	bool ListOp::UpdateIndex(ListPolicy policy, Modifiable<size_t>& idx) const
	{
		switch (type)
		{
		case ListOpType::Create:
			break;

		case ListOpType::Delete:
			if (idx == GetIndex())
				return false;

			if (idx > GetIndex())
				idx = idx - 1;

			break;

		case ListOpType::Resize:
			if (idx >= GetNewSize())
				return false;

			break;

		case ListOpType::Clear:
			return false;

			break;

		case ListOpType::Move:
		{
			size_t min = std::min(GetSrcIndex(), GetDstIndex());
			size_t max = std::max(GetSrcIndex(), GetDstIndex());

			if (idx >= min && idx <= max)
			{
				if (idx == GetSrcIndex())
					idx = GetDstIndex();
				else if (GetSrcIndex() < GetDstIndex())
					idx = idx - 1;
				else
					idx = idx + 1;
			}

			break;
		}
		}

		return true;
	}

	bool ListOp::UpdateIndex(ListPolicy policy, ListOp& op) const
	{
		return UpdateIndex(policy, op.index1) && UpdateIndex(policy, op.index2);
	}

	void ListModel::Init(IListAdapter& adapter)
	{
		active_index = 0;
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

	// TODO v9.1 undo actions for ListModel operations

	void ListModel::DeferCreate()
	{
		_pending_ops.push_back(ListOp::MakeCreateOp());
	}
	
	void ListModel::DeferDelete()
	{
		_pending_ops.push_back(ListOp::MakeDeleteOp(active_index));
	}
	
	void ListModel::DeferResize(size_t new_size)
	{
		_pending_ops.push_back(ListOp::MakeResizeOp(_size, new_size));
	}

	void ListModel::DeferClear()
	{
		_pending_ops.push_back(ListOp::MakeClearOp());
	}

	bool ListModel::ConsumeOps(IListAdapter& adapter)
	{
		bool any = false;

		for (auto it = _pending_ops.begin(); it != _pending_ops.end(); ++it)
		{
			if (!it->valid)
				continue;

			any = true;
			Apply(*it, adapter);

			for (auto ut = std::next(it); ut != _pending_ops.end(); )
			{
				ut->Validate(ut->valid && it->UpdateIndex(policy, *ut));
				if (ut->valid)
					++ut;
				else
					ut = _pending_ops.erase(ut);
			}
		}

		_pending_ops.clear();
		return any;
	}

	void ListModel::Apply(const ListOp& op, IListAdapter& adapter)
	{
		switch (op.type)
		{
		case ListOpType::Create:
			++_size;
			SetLast();
			adapter.PushBack();
			break;

		case ListOpType::Delete:
			if (_size > 0)
			{
				--_size;
				adapter.Erase(op.GetIndex());
			}
			break;

		case ListOpType::Resize:
			_size = op.GetNewSize();
			adapter.Resize(op.GetOldSize(), _size);
			break;

		case ListOpType::Clear:
			_size = 0;
			adapter.Clear();
			break;

		case ListOpType::Move:
			adapter.Move(op.GetSrcIndex(), op.GetDstIndex());
			break;
		}

		if (!op.UpdateIndex(policy, active_index))
			Clamp();

		EnforcePolicy(adapter);
	}

	void ListModel::EnforcePolicy(IListAdapter& adapter)
	{
		if (_size == 0 && policy == ListPolicy::MinimumOne)
			Apply(ListOp::MakeCreateOp(), adapter);
	}

	void ListModel::Invoke(const ListOp& op, IListAdapter& adapter)
	{
		if (!_pending_ops.empty())
			ConsumeOps(adapter);

		Apply(op, adapter);
	}

	DrawResult ListModel::DrawComboHeader(const char* slot_prefix, const char* create_tooltip, const char* delete_tooltip, const char* clear_tooltip)
	{
		return DrawComboHeader([slot_prefix](size_t i) { return slot_prefix + (" " + std::to_string(i)); }, create_tooltip, delete_tooltip, clear_tooltip);
	}

	DrawResult ListModel::DrawComboHeader(std::function<std::string(size_t)> combo_getter, const char* create_tooltip, const char* delete_tooltip, const char* clear_tooltip)
	{
		DrawResult result;
		DrawResult subresult;

		std::vector<std::string> slot_names;
		slot_names.reserve(_size);
		for (int i = 0; i < _size; ++i)
			slot_names.push_back(combo_getter(i));

		int slot = active_index;
		result |= gui::Combo("##SelectSlot", slot, slot_names);
		active_index = slot;

		ImGui::SameLine();
		subresult = Toolbar::DrawIconButton(IconResource::Plus, create_tooltip, "##+");
		result |= subresult;
		if (subresult)
			DeferCreate();

		ImGui::SameLine();
		subresult = Toolbar::DrawIconButton(IconResource::Minus, delete_tooltip, "##-");
		result |= subresult;
		if (subresult)
			DeferDelete();

		ImGui::SameLine();
		subresult = Toolbar::DrawIconButton(IconResource::Close, clear_tooltip, "##x");
		result |= subresult;
		if (subresult)
			DeferClear();

		return result;
	}
}
