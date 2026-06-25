#pragma once

#include "core/UndoHistory.h"
#include "desc/DataPath.h"

namespace oly::editor
{
	// TODO v9.1 log warnings if visitor returns nullptr -> remove earlier undo actions + self from history (break stack). Forward()/Backward() should return true if successful, so UndoHistory can manage removeing failed actions that return false.

	template<typename T>
	struct FieldSetAction : public UndoAction
	{
		DataPathSource path;
		T initial_value;
		T final_value;

		FieldSetAction(DataPath path, T initial_value, T final_value) :
			path(path.Copy()), initial_value(std::move(initial_value)), final_value(std::move(final_value))
		{
		}

		void Forward() override
		{
			if (void* var = DataPathVisitor::ActiveInstance()(path, typeid(T)))
			{
				T& ref = *reinterpret_cast<T*>(var);
				ref = final_value;
			}
		}

		void Backward() override
		{
			if (void* var = DataPathVisitor::ActiveInstance()(path, typeid(T)))
			{
				T& ref = *reinterpret_cast<T*>(var);
				ref = initial_value;
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
		UndoHistory::ActiveInstance().Push(std::make_unique<FieldSetAction<T>>(path, initial_value, final_value));
	}

	template<typename T>
	struct FieldSyncSetAction : public UndoAction
	{
		DataPathSource path;
		T initial_value;
		T final_value;
		std::function<void()> sync;

		FieldSyncSetAction(DataPath path, T initial_value, T final_value, std::function<void()> sync) :
			path(path.Copy()), initial_value(std::move(initial_value)), final_value(std::move(final_value)), sync(std::move(sync))
		{
		}

		void Forward() override
		{
			if (void* var = DataPathVisitor::ActiveInstance()(path, typeid(T)))
			{
				T& ref = *reinterpret_cast<T*>(var);
				ref = final_value;
				sync();
			}
		}

		void Backward() override
		{
			if (void* var = DataPathVisitor::ActiveInstance()(path, typeid(T)))
			{
				T& ref = *reinterpret_cast<T*>(var);
				ref = initial_value;
				sync();
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
		UndoHistory::ActiveInstance().Push(std::make_unique<FieldSyncSetAction<T>>(path, initial_value, final_value, std::move(sync)));
	}
}
