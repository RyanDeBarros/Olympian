#pragma once

#include "core/UndoHistory.h"

namespace oly::editor
{
	template<std::derived_from<UndoAction> Action>
	struct ReversedUndoAction : public Action
	{
		bool Forward() override
		{
			return Action::Backward();
		}
		
		bool Backward() override
		{
			return Action::Forward();
		}
	};

	struct CompoundUndoAction : public UndoAction
	{
		std::vector<std::unique_ptr<UndoAction>> forward_queue;

		bool Forward() override;
		bool Backward() override;
		size_t EmpiricalSize() const override;
	};

	class CompoundUndoActionQueue
	{
		struct Entry
		{
			std::unique_ptr<UndoAction> _action;
			bool _execute_on_push;
		};

		std::vector<Entry> _entries;

	public:
		void Append(std::unique_ptr<UndoAction>&& action, bool execute_on_push);
		void PushAll(UndoHistory& undo_history);
	};
}
