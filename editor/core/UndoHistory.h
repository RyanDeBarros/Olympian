#pragma once

#include "util/FunctionalEvent.h"

#include <memory>
#include <optional>
#include <vector>

namespace oly::editor
{
	struct UndoAction
	{
		virtual ~UndoAction() = default;

		virtual bool Forward() = 0;
		virtual bool Backward() = 0;
		virtual size_t EmpiricalSize() const = 0;
	};

	class UndoHistory
	{
		std::vector<std::unique_ptr<UndoAction>> _undo;
		size_t _undo_stack_size = 0;
		std::vector<std::unique_ptr<UndoAction>> _redo;
		size_t _redo_stack_size = 0;
		FunctionalEvent<>::Handle _listener;
		std::optional<size_t> _clean_marker;

	public:
		UndoHistory();

		static UndoHistory& ActiveInstance();

		void Execute(std::unique_ptr<UndoAction>&& action);
		void Push(std::unique_ptr<UndoAction>&& action);

		void Undo();
		void Redo();
		void MarkClean();

		void Prune();
		void Clear();

	private:
		void PruneUndoCount(size_t count_limit);
		void PruneUndoSize(size_t size_limit);
	};

	struct UndoHistoryActiveScope
	{
		UndoHistoryActiveScope(UndoHistory& undo_history);
		UndoHistoryActiveScope(const UndoHistoryActiveScope&) = delete;
		UndoHistoryActiveScope(UndoHistoryActiveScope&&) = delete;
		~UndoHistoryActiveScope();
	};
}
