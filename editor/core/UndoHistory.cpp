#include "UndoHistory.h"

#include <stack>

// TODO LATER popup inside document window to view list of undo actions so you can click on a certain action to rollback/forward to

namespace oly::editor
{
	UndoHistory::UndoHistory(size_t count_limit, size_t size_limit)
		: _count_limit(count_limit), _size_limit(size_limit)
	{
	}

	UndoHistory& UndoHistory::ActiveInstance()
	{
		return ActiveUndoHistory::instance().uh_instance;
	}

	void UndoHistory::Execute(std::unique_ptr<UndoAction>&& action)
	{
		if (action->Forward())
			Push(std::move(action));
		else
			Clear();
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

			std::unique_ptr<UndoAction> action = std::move(_undo.back());
			_undo.pop_back();

			if (action->Backward())
			{
				_redo.push_back(std::move(action));
				OnUndoPostSuccess();
			}
			else
			{
				OnUndoPreFail();
				_undo_stack_size = 0;
				_undo.clear();
			}

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

			std::unique_ptr<UndoAction> action = std::move(_redo.back());
			_redo.pop_back();

			if (action->Forward())
			{
				_undo.push_back(std::move(action));
				OnRedoPostSuccess();
			}
			else
			{
				OnRedoPreFail();
				_redo_stack_size = 0;
				_redo.clear();
			}

			Prune();
		}
	}

	void UndoHistory::Prune()
	{
		if (_redo.size() >= _count_limit)
			PruneUndoCount(0);
		else
			PruneUndoCount(_count_limit - _redo.size());

		if (_redo_stack_size >= _size_limit)
			PruneUndoSize(0);
		else
			PruneUndoSize(_size_limit - _redo_stack_size);
	}

	void UndoHistory::Clear()
	{
		_undo_stack_size = 0;
		_undo.clear();

		_redo_stack_size = 0;
		_redo.clear();
	}

	void UndoHistory::SetLimits(size_t count_limit, size_t size_limit)
	{
		_count_limit = count_limit;
		_size_limit = size_limit;
		Prune();
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

	size_t UndoHistory::UndoCount() const
	{
		return _undo.size();
	}

	ActiveUndoHistory::ActiveUndoHistory(UndoHistory& undo_history)
		: uh_instance(undo_history)
	{
	}

	void CheckpointUndoHistory::Push(std::unique_ptr<UndoAction>&& action)
	{
		if (_clean_marker && *_clean_marker > UndoCount())
			_clean_marker.reset();

		UndoHistory::Push(std::move(action));
	}

	void CheckpointUndoHistory::Prune()
	{
		const size_t initial_undo_count = UndoCount();

		UndoHistory::Prune();

		if (_clean_marker)
		{
			size_t delta = initial_undo_count - UndoCount();

			if (*_clean_marker >= delta)
				_clean_marker = *_clean_marker - delta;
			else
				_clean_marker.reset();
		}
	}

	void CheckpointUndoHistory::Clear()
	{
		UndoHistory::Clear();

		if (_clean_marker && *_clean_marker > 0)
			_clean_marker.reset();
	}

	void CheckpointUndoHistory::MarkClean()
	{
		_clean_marker = UndoCount();
	}

	void CheckpointUndoHistory::OnUndoPostSuccess()
	{
		if (_clean_marker && (UndoCount() == *_clean_marker || UndoCount() + 1 == *_clean_marker))
			on_potential_clean.invoke();
	}
	
	void CheckpointUndoHistory::OnUndoPreFail()
	{
		if (_clean_marker)
		{
			if (*_clean_marker <= UndoCount())
				_clean_marker.reset();
			else
				_clean_marker = *_clean_marker - (UndoCount() + 1);
		}
	}
	
	void CheckpointUndoHistory::OnRedoPostSuccess()
	{
		if (_clean_marker && (UndoCount() == *_clean_marker || UndoCount() == *_clean_marker + 1))
			on_potential_clean.invoke();
	}
	
	void CheckpointUndoHistory::OnRedoPreFail()
	{
		if (_clean_marker && *_clean_marker > UndoCount())
			_clean_marker.reset();
	}
}
