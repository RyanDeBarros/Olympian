#include "UndoHistory.h"

namespace oly::editor
{
	UndoHistory::UndoHistory() :
		_stack_count_limit(500),  // TODO v9.1 use preferences setting (default 500)
		_stack_size_limit(4 * 1024 * 1024)  // TODO v9.1 use preferences setting (default 32MiB)
	{
	}

	void UndoHistory::Execute(std::unique_ptr<UndoAction>&& action)
	{
		_undo_stack_size += action->EmpiricalSize();
		_redo_stack_size = 0;

		action->Forward();
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
		if (_redo.size() >= _stack_count_limit)
			PruneUndoCount(0);
		else
			PruneUndoCount(_stack_count_limit - _redo.size());

		if (_redo_stack_size >= _stack_size_limit)
			PruneUndoSize(0);
		else
			PruneUndoSize(_stack_size_limit - _redo_stack_size);
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
}
