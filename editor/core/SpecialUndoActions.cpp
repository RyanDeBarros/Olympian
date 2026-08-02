#include "SpecialUndoActions.h"

namespace oly::editor
{
	bool CompoundUndoAction::Forward()
	{
		bool success = true;
		for (auto it = forward_queue.begin(); it != forward_queue.end(); ++it)
			success &= (*it)->Forward();
		return true;
	}

	bool CompoundUndoAction::Backward()
	{
		bool success = true;
		for (auto it = forward_queue.rbegin(); it != forward_queue.rend(); ++it)
			success &= (*it)->Backward();
		return true;
	}
	
	size_t CompoundUndoAction::EmpiricalSize() const
	{
		size_t size = sizeof(*this);
		for (const auto& action : forward_queue)
			size += action->EmpiricalSize();
		return size;
	}

	void CompoundUndoActionQueue::Append(std::unique_ptr<UndoAction>&& action, bool execute_on_push)
	{
		_entries.emplace_back(std::move(action), execute_on_push);
	}

	void CompoundUndoActionQueue::PushAll(UndoHistory& undo_history)
	{
		if (!_entries.empty())
		{
			auto batch = std::make_unique<CompoundUndoAction>();
			for (auto& entry : _entries)
			{
				if (entry._execute_on_push)
					entry._action->Forward();

				batch->forward_queue.push_back(std::move(entry._action));
			}
			undo_history.Push(std::move(batch));
			_entries.clear();
		}
	}
}
