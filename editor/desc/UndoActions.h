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

		bool Forward() override
		{
			if (void* var = DataPathVisitor::ActiveInstance()(path, typeid(T)))
			{
				T& ref = *reinterpret_cast<T*>(var);
				ref = final_value;
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
			if (void* var = DataPathVisitor::ActiveInstance()(path, typeid(T)))
			{
				T& ref = *reinterpret_cast<T*>(var);
				ref = initial_value;
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

		bool Forward() override
		{
			if (void* var = DataPathVisitor::ActiveInstance()(path, typeid(T)))
			{
				T& ref = *reinterpret_cast<T*>(var);
				ref = final_value;
				sync();
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
			if (void* var = DataPathVisitor::ActiveInstance()(path, typeid(T)))
			{
				T& ref = *reinterpret_cast<T*>(var);
				ref = initial_value;
				sync();
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
		UndoHistory::ActiveInstance().Push(std::make_unique<FieldSyncSetAction<T>>(path, initial_value, final_value, std::move(sync)));
	}
}
