#pragma once

#include "util/FunctionalEvent.h"

#include <memory>
#include <optional>
#include <vector>

#include <imtk.hpp>

// TODO v9.3 move to imp, not imtk. must move instance_guard/stack to imp first though

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
		size_t _count_limit;
		size_t _size_limit;

	public:
		UndoHistory(size_t count_limit, size_t size_limit);
		virtual ~UndoHistory() = default;

		static UndoHistory& ActiveInstance();

		void Execute(std::unique_ptr<UndoAction>&& action);

		virtual void Push(std::unique_ptr<UndoAction>&& action);

		void Undo();
		void Redo();

		virtual void Prune();
		virtual void Clear();
		
		void SetLimits(size_t count_limit, size_t size_limit);

	private:
		void PruneUndoCount(size_t count_limit);
		void PruneUndoSize(size_t size_limit);

	protected:
		size_t UndoCount() const;

		virtual void OnUndoPostSuccess() {};
		virtual void OnUndoPreFail() {};
		virtual void OnRedoPostSuccess() {};
		virtual void OnRedoPreFail() {};
	};

	struct ActiveUndoHistory : public imtk::instance_stack<ActiveUndoHistory>
	{
		UndoHistory& uh_instance;

		ActiveUndoHistory(UndoHistory& undo_history);
	};

	class CheckpointUndoHistory : public UndoHistory
	{
		std::optional<size_t> _clean_marker;

	public:
		FunctionalEvent<> on_potential_clean;

		using UndoHistory::UndoHistory;

		void Push(std::unique_ptr<UndoAction>&& action) override;
		void Prune() override;
		void Clear() override;

		void MarkClean();

	protected:
		void OnUndoPostSuccess() override;
		void OnUndoPreFail() override;
		void OnRedoPostSuccess() override;
		void OnRedoPreFail() override;
	};
}
