#pragma once

#include <imp/undo_history.hpp>

namespace oly::editor
{
	template<std::derived_from<imp::undo_action> Action>
	struct ReversedUndoAction : public Action
	{
		bool forward() override
		{
			return Action::backward();
		}
		
		bool backward() override
		{
			return Action::forward();
		}
	};

	struct CompoundUndoAction : public imp::undo_action
	{
		std::vector<std::unique_ptr<imp::undo_action>> forward_queue;

		bool forward() override;
		bool backward() override;
		size_t empirical_size() const override;
	};

	class CompoundUndoActionQueue
	{
		struct Entry
		{
			std::unique_ptr<imp::undo_action> _action;
			bool _execute_on_push;
		};

		std::vector<Entry> _entries;

	public:
		void Append(std::unique_ptr<imp::undo_action>&& action, bool execute_on_push);
		void PushAll(imp::undo_history& undo_history);
	};
}
