#include "UndoHistory.h"

#include "core/editor/Editor.h"
#include "core/Errors.h"

#include "desc/impl/PreferencesDesc.h"

#include <stack>

namespace oly::editor
{
	static std::stack<UndoHistory*> UNDO_HISTORY_STACK;

	UndoHistory::UndoHistory()
	{
		_listener = Editor::Instance().OnPreferencesChanged.subscribe([this]() { Prune(); });
	}

	UndoHistory& UndoHistory::ActiveInstance()
	{
		if (UNDO_HISTORY_STACK.empty())
			BreakoutError::Throw("UndoHistory::ActiveInstance(): live undo history stack is empty");
		else
			return *UNDO_HISTORY_STACK.top();
	}

	void UndoHistory::Execute(std::unique_ptr<UndoAction>&& action)
	{
		action->Forward();
		Push(std::move(action));
	}

	void UndoHistory::Push(std::unique_ptr<UndoAction>&& action)
	{
		_undo_stack_size += action->EmpiricalSize();
		_redo_stack_size = 0;

		_undo.push_back(std::move(action));
		_redo.clear();
		Prune();
	}

	void UndoHistory::Undo()
	{
		if (!_undo.empty())
		{
			const size_t sz = _undo.back()->EmpiricalSize();
			_undo_stack_size -= sz;
			_redo_stack_size += sz;

			_undo.back()->Backward();
			_redo.push_back(std::move(_undo.back()));
			_undo.pop_back();

			Prune();
		}
	}

	void UndoHistory::Redo()
	{
		if (!_redo.empty())
		{
			const size_t sz = _redo.back()->EmpiricalSize();
			_redo_stack_size -= sz;
			_undo_stack_size += sz;

			_redo.back()->Forward();
			_undo.push_back(std::move(_redo.back()));
			_redo.pop_back();

			Prune();
		}
	}

	void UndoHistory::Prune()
	{
		const size_t count_limit = Editor::GetPreferences().edit.undo_history.CountLimit();
		if (_redo.size() >= count_limit)
			PruneUndoCount(0);
		else
			PruneUndoCount(count_limit - _redo.size());

		const size_t size_limit = Editor::GetPreferences().edit.undo_history.SizeLimit();
		if (_redo_stack_size >= size_limit)
			PruneUndoSize(0);
		else
			PruneUndoSize(size_limit - _redo_stack_size);
	}

	void UndoHistory::PruneUndoCount(size_t count_limit)
	{
		if (count_limit == 0)
		{
			_undo_stack_size = 0;
			_undo.clear();
		}
		else
		{
			if (_undo.size() > count_limit)
			{
				const size_t amount = _undo.size() - count_limit;
				
				for (size_t i = 0; i < amount; ++i)
					_undo_stack_size -= _undo[i]->EmpiricalSize();

				_undo.erase(_undo.begin(), _undo.begin() + amount);
			}
		}
	}

	void UndoHistory::PruneUndoSize(size_t size_limit)
	{
		if (size_limit == 0)
		{
			_undo_stack_size = 0;
			_undo.clear();
		}
		else
		{
			auto it = _undo.begin();
			while (_undo_stack_size > size_limit && it != _undo.end())
				_undo_stack_size -= (*it++)->EmpiricalSize();

			if (it != _undo.begin())
				_undo.erase(_undo.begin(), it);
		}
	}

	UndoHistoryActiveScope::UndoHistoryActiveScope(UndoHistory& undo_history)
	{
		UNDO_HISTORY_STACK.push(&undo_history);
	}

	UndoHistoryActiveScope::~UndoHistoryActiveScope()
	{
		UNDO_HISTORY_STACK.pop();
	}
}
