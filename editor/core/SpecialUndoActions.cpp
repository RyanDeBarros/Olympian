#include "SpecialUndoActions.h"

namespace oly::editor
{
	bool CompoundUndoAction::forward()
	{
		bool success = true;
		for (auto it = forward_queue.begin(); it != forward_queue.end(); ++it)
			success &= (*it)->forward();
		return true;
	}

	bool CompoundUndoAction::backward()
	{
		bool success = true;
		for (auto it = forward_queue.rbegin(); it != forward_queue.rend(); ++it)
			success &= (*it)->backward();
		return true;
	}
	
	size_t CompoundUndoAction::empirical_size() const
	{
		size_t size = sizeof(*this);
		for (const auto& action : forward_queue)
			size += action->empirical_size();
		return size;
	}

	void CompoundUndoActionQueue::Append(std::unique_ptr<imp::undo_action>&& action, bool execute_on_push)
	{
		_entries.emplace_back(std::move(action), execute_on_push);
	}

	void CompoundUndoActionQueue::PushAll(imp::undo_history& undo_history)
	{
		if (!_entries.empty())
		{
			auto batch = std::make_unique<CompoundUndoAction>();
			for (auto& entry : _entries)
			{
				if (entry._execute_on_push)
					entry._action->forward();

				batch->forward_queue.push_back(std::move(entry._action));
			}
			undo_history.push(std::move(batch));
			_entries.clear();
		}
	}
}
