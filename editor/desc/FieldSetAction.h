#pragma once

#include "core/Printer.h"
#include "core/UndoHistory.h"
#include "core/editor/Logger.h"

#include "documents/ActiveDocument.h"
#include "documents/IDocument.h"

#include "desc/DataPath.h"

#include <sstream>

namespace oly::editor
{
	template<typename T, typename Printer = StandardPrinter<T>>
	struct FieldSetAction : public UndoAction
	{
		DataPathSource path;
		T initial_value;
		T final_value;

		FieldSetAction(DataPath path, T initial_value, T final_value) :
			path(path), initial_value(std::move(initial_value)), final_value(std::move(final_value))
		{
		}

		bool Forward() override
		{
			bool success = false;
			if (void* var = ActiveDocument::Get().PathGet(path, typeid(T)))
			{
				T& ref = *static_cast<T*>(var);
				ref = final_value;
				success = true;
			}

			std::stringstream ss;
			ss << "Redo action " << (success ? "success" : "fail") << ": [path=" << ActiveDocument::Get().PathString(path);
			if constexpr (!std::is_void_v<Printer>)
			{
				ss << ", from=";
				Printer{}(ss, initial_value);
				ss << ", to=";
				Printer{}(ss, final_value);
			}
			ss << "]";
			Logger::Instance().Log(success ? LogLevel::Success : LogLevel::Error, ss.str());

			return success;
		}

		bool Backward() override
		{
			bool success = false;
			if (void* var = ActiveDocument::Get().PathGet(path, typeid(T)))
			{
				T& ref = *static_cast<T*>(var);
				ref = initial_value;
				success = true;
			}

			std::stringstream ss;
			ss << "Undo action " << (success ? "success" : "fail") << ": [path=" << ActiveDocument::Get().PathString(path);
			if constexpr (!std::is_void_v<Printer>)
			{
				ss << ", from=";
				Printer{}(ss, final_value);
				ss << ", to=";
				Printer{}(ss, initial_value);
			}
			ss << "]";
			Logger::Instance().Log(success ? LogLevel::Success : LogLevel::Error, ss.str());

			return success;
		}

		size_t EmpiricalSize() const
		{
			return sizeof(*this);
		}
	};

	template<typename T, typename Printer = StandardPrinter<T>>
	void PushFieldSetAction(DataPath path, T initial_value, T final_value)
	{
		UndoHistory::ActiveInstance().Push(std::make_unique<FieldSetAction<T, Printer>>(path, std::move(initial_value), std::move(final_value)));
	}
}
