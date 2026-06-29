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
}
