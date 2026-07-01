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
}
