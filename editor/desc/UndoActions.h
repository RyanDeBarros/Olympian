#pragma once

#include "core/UndoHistory.h"
#include "core/editor/Logger.h"

#include "documents/ActiveDocument.h"
#include "documents/IDocument.h"

#include "desc/DataPath.h"

#include "util/CommonOStream.h"

#include <sstream>

namespace oly::editor
{
	namespace internal
	{
		template<typename T>
		void LogUndoActionSuccess(bool undo, DataPath path, const T& initial_value, const T& final_value)
		{
			std::stringstream ss;
			if (undo)
				ss << "Undo";
			else
				ss << "Redo";
			ss << " action success: [path=" << ActiveDocument::Get().PathString(path) << ", from=" << initial_value << ", to=" << final_value << "]";
			Logger::Instance().Log(LogLevel::Success, ss.str());
		}
		
		inline void LogUndoActionSuccess(bool undo, DataPath path)
		{
			std::stringstream ss;
			if (undo)
				ss << "Undo";
			else
				ss << "Redo";
			ss << " action success: [path=" << ActiveDocument::Get().PathString(path) << "]";
			Logger::Instance().Log(LogLevel::Success, ss.str());
		}

		template<typename T>
		void LogUndoActionFail(bool undo, DataPath path, const T& initial_value, const T& final_value)
		{
			std::stringstream ss;
			if (undo)
				ss << "Undo";
			else
				ss << "Redo";
			ss << " action failed: [path=" << ActiveDocument::Get().PathString(path) << ", from=" << initial_value << ", to=" << final_value << "]";
			Logger::Instance().Log(LogLevel::Warning, ss.str());
		}

		inline void LogUndoActionFail(bool undo, DataPath path)
		{
			std::stringstream ss;
			if (undo)
				ss << "Undo";
			else
				ss << "Redo";
			ss << " action failed: [path=" << ActiveDocument::Get().PathString(path) << "]";
			Logger::Instance().Log(LogLevel::Warning, ss.str());
		}
	}

	template<typename T, bool PrintableValue = true>
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
			if (void* var = ActiveDocument::Get().PathGet(path, typeid(T)))
			{
				T& ref = *reinterpret_cast<T*>(var);
				ref = final_value;

				if constexpr (PrintableValue)
					internal::LogUndoActionSuccess(false, path, initial_value, final_value);
				else
					internal::LogUndoActionSuccess(false, path);

				return true;
			}
			else
			{
				if constexpr (PrintableValue)
					internal::LogUndoActionFail(false, path, initial_value, final_value);
				else
					internal::LogUndoActionFail(false, path);

				return false;
			}
		}

		bool Backward() override
		{
			if (void* var = ActiveDocument::Get().PathGet(path, typeid(T)))
			{
				T& ref = *reinterpret_cast<T*>(var);
				ref = initial_value;

				if constexpr (PrintableValue)
					internal::LogUndoActionSuccess(true, path, final_value, initial_value);
				else
					internal::LogUndoActionSuccess(true, path);

				return true;
			}
			else
			{
				if constexpr (PrintableValue)
					internal::LogUndoActionFail(true, path, final_value, initial_value);
				else
					internal::LogUndoActionFail(true, path);

				return false;
			}
		}

		size_t EmpiricalSize() const
		{
			return sizeof(*this);
		}
	};

	template<typename T, bool PrintableValue = true>
	void PushFieldSetAction(DataPath path, T initial_value, T final_value)
	{
		UndoHistory::ActiveInstance().Push(std::make_unique<FieldSetAction<T, PrintableValue>>(path, std::move(initial_value), std::move(final_value)));
	}
}
