#pragma once

#include "core/UndoHistory.h"
#include "core/editor/Logger.h"

#include "desc/DataPath.h"

#include "util/CommonOStream.h"

namespace oly::editor
{
	namespace internal
	{
		template<typename T>
		void LogUndoActionFail(const char* function, DataPath path, const T& initial_value, const T& final_value)
		{
			std::stringstream ss;
			ss << function << " failed: [path=" << path << ", initial_value=" << initial_value << ", final_value=" << final_value << "]";
			Logger::Instance().Log(LogLevel::Warning, ss.str());
		}
	}

	// TODO v9.1 also handle case of Forward() -> Save() -> Backward() -> Forward(). At this point, call doc.QueryDirty() again since it should be clean from the previous save. Basically, call query_dirty() if doc is clean in the before state OR the after state, in both Forward() and Backward().
	// TODO v9.1 undo action for reload/revert asset if it changed anything.
	// TODO v9.1 log successful undo/redo actions

	template<typename T>
	struct FieldSetAction : public UndoAction
	{
		DataPathSource path;
		T initial_value;
		T final_value;
		bool initially_dirty;

		FieldSetAction(DataPath path, T initial_value, T final_value) :
			path(path.Copy()), initial_value(std::move(initial_value)), final_value(std::move(final_value)), initially_dirty(DataPathVisitor::ActiveInstance().is_dirty())
		{
		}

		bool Forward() override
		{
			if (void* var = DataPathVisitor::ActiveInstance().visit_path(path, typeid(T)))
			{
				T& ref = *reinterpret_cast<T*>(var);
				ref = final_value;

				if (!initially_dirty)
					DataPathVisitor::ActiveInstance().query_dirty();

				return true;
			}
			else
			{
				internal::LogUndoActionFail(__FUNCTION__, path, initial_value, final_value);
				return false;
			}
		}

		bool Backward() override
		{
			if (void* var = DataPathVisitor::ActiveInstance().visit_path(path, typeid(T)))
			{
				T& ref = *reinterpret_cast<T*>(var);
				ref = initial_value;

				if (!initially_dirty)
					DataPathVisitor::ActiveInstance().query_dirty();

				return true;
			}
			else
			{
				internal::LogUndoActionFail(__FUNCTION__, path, initial_value, final_value);
				return false;
			}
		}

		size_t EmpiricalSize() const
		{
			return sizeof(*this);
		}
	};

	template<typename T>
	void PushFieldSetAction(DataPath path, T initial_value, T final_value)
	{
		UndoHistory::ActiveInstance().Push(std::make_unique<FieldSetAction<T>>(path, std::move(initial_value), std::move(final_value)));
	}

	template<typename T>
	struct FieldSyncSetAction : public UndoAction
	{
		DataPathSource path;
		T initial_value;
		T final_value;
		std::function<void()> sync;
		bool initially_dirty;

		FieldSyncSetAction(DataPath path, T initial_value, T final_value, std::function<void()> sync) :
			path(path.Copy()), initial_value(std::move(initial_value)), final_value(std::move(final_value)), sync(std::move(sync)), initially_dirty(DataPathVisitor::ActiveInstance().is_dirty())
		{
		}

		bool Forward() override
		{
			if (void* var = DataPathVisitor::ActiveInstance().visit_path(path, typeid(T)))
			{
				T& ref = *reinterpret_cast<T*>(var);
				ref = final_value;
				sync();

				if (!initially_dirty)
					DataPathVisitor::ActiveInstance().query_dirty();

				return true;
			}
			else
			{
				internal::LogUndoActionFail(__FUNCTION__, path, initial_value, final_value);
				return false;
			}
		}

		bool Backward() override
		{
			if (void* var = DataPathVisitor::ActiveInstance().visit_path(path, typeid(T)))
			{
				T& ref = *reinterpret_cast<T*>(var);
				ref = initial_value;
				sync();

				if (!initially_dirty)
					DataPathVisitor::ActiveInstance().query_dirty();

				return true;
			}
			else
			{
				internal::LogUndoActionFail(__FUNCTION__, path, initial_value, final_value);
				return false;
			}
		}

		size_t EmpiricalSize() const
		{
			return sizeof(*this);
		}
	};

	template<typename T>
	void PushFieldSyncSetAction(DataPath path, T initial_value, T final_value, std::function<void()> sync)
	{
		UndoHistory::ActiveInstance().Push(std::make_unique<FieldSyncSetAction<T>>(path, std::move(initial_value), std::move(final_value), std::move(sync)));
	}
}
