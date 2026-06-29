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
	template<typename ElementType, bool PrintableValue = true>
	struct DynamicListDeleteAction : public UndoAction
	{
		using ListType = std::vector<ElementType>;

		DataPathSource list_path;
		size_t delete_index;
		ElementType deleted_element;

		DynamicListDeleteAction(DataPath list_path, size_t delete_index, ElementType deleted_element)
			: list_path(list_path), delete_index(delete_index), deleted_element(std::move(deleted_element))
		{
		}

		bool Forward() override
		{
			bool success = false;
			if (void* var = ActiveDocument::Get().PathGet(list_path, typeid(ListType)))
			{
				auto& ref_vector = *static_cast<ListType*>(var);
				if (delete_index < ref_vector.size())
				{
					deleted_element = std::move(ref_vector[delete_index]);
					ref_vector.erase(ref_vector.begin() + delete_index);
					success = true;
				}
			}

			std::stringstream ss;
			ss << "Redo action " << (success ? "success" : "fail") << ": [path=" << ActiveDocument::Get().PathString(list_path) << ", delete@index=" << delete_index;
			if (PrintableValue)
				ss << ", delete@element=" << deleted_element;
			ss << "]";
			Logger::Instance().Log(success ? LogLevel::Success : LogLevel::Error, ss.str());

			return success;
		}

		bool Backward() override
		{
			bool success = false;
			if (void* var = ActiveDocument::Get().PathGet(list_path, typeid(ListType)))
			{
				auto& ref_vector = *static_cast<ListType*>(var);
				if (delete_index <= ref_vector.size())
				{
					ref_vector.insert(ref_vector.begin() + delete_index, deleted_element);
					success = true;
				}
			}

			std::stringstream ss;
			ss << "Undo action " << (success ? "success" : "fail") << ": [path=" << ActiveDocument::Get().PathString(list_path) << ", re-insert@index=" << delete_index;
			if (PrintableValue)
				ss << ", re-insert@element=" << deleted_element;
			ss << "]";
			Logger::Instance().Log(success ? LogLevel::Success : LogLevel::Error, ss.str());

			return success;
		}

		size_t EmpiricalSize() const override
		{
			return sizeof(*this);
		}
	};

	template<typename ElementType, bool PrintableValue = true>
	void ExecuteDynamicListDeleteAction(DataPath list_path, size_t delete_index)
	{
		UndoHistory::ActiveInstance().Execute(std::make_unique<DynamicListDeleteAction<ElementType, PrintableValue>>(list_path, delete_index, ElementType{}));
	}

	template<typename ElementType, bool PrintableValue = true>
	struct DynamicListInsertAction : public UndoAction
	{
		using ListType = std::vector<ElementType>;

		DataPathSource list_path;
		size_t insert_index;
		ElementType inserted_element;

		DynamicListInsertAction(DataPath list_path, size_t insert_index, ElementType inserted_element)
			: list_path(list_path), insert_index(insert_index), inserted_element(std::move(inserted_element))
		{
		}

		bool Forward() override
		{
			bool success = false;
			if (void* var = ActiveDocument::Get().PathGet(list_path, typeid(ListType)))
			{
				auto& ref_vector = *static_cast<ListType*>(var);
				if (insert_index <= ref_vector.size())
				{
					ref_vector.insert(ref_vector.begin() + insert_index, inserted_element);
					success = true;
				}
			}

			std::stringstream ss;
			ss << "Redo action " << (success ? "success" : "fail") << ": [path=" << ActiveDocument::Get().PathString(list_path) << ", insert@index=" << insert_index;
			if (PrintableValue)
				ss << ", insert@element=" << inserted_element;
			ss << "]";
			Logger::Instance().Log(success ? LogLevel::Success : LogLevel::Error, ss.str());

			return success;
		}

		bool Backward() override
		{
			bool success = false;
			if (void* var = ActiveDocument::Get().PathGet(list_path, typeid(ListType)))
			{
				auto& ref_vector = *static_cast<ListType*>(var);
				if (insert_index < ref_vector.size())
				{
					inserted_element = std::move(ref_vector[insert_index]);
					ref_vector.erase(ref_vector.begin() + insert_index);
					success = true;
				}
			}

			std::stringstream ss;
			ss << "Undo action " << (success ? "success" : "fail") << ": [path=" << ActiveDocument::Get().PathString(list_path) << ", re-delete@index=" << insert_index;
			if (PrintableValue)
				ss << ", re-delete@element=" << inserted_element;
			ss << "]";
			Logger::Instance().Log(success ? LogLevel::Success : LogLevel::Error, ss.str());

			return success;
		}

		size_t EmpiricalSize() const override
		{
			return sizeof(*this);
		}
	};

	template<typename ElementType, bool PrintableValue = true>
	void ExecuteDynamicListInsertAction(DataPath list_path, size_t insert_index)
	{
		UndoHistory::ActiveInstance().Execute(std::make_unique<DynamicListInsertAction<ElementType, PrintableValue>>(list_path, insert_index, ElementType{}));
	}

	template<typename ElementType>
	struct DynamicListMoveAction : public UndoAction
	{
		using ListType = std::vector<ElementType>;

		DataPathSource list_path;
		size_t src_index;
		size_t dst_index;

		DynamicListMoveAction(DataPath list_path, size_t src_index, size_t dst_index)
			: list_path(list_path), src_index(src_index), dst_index(dst_index)
		{
		}

		bool Forward() override
		{
			bool success = false;
			if (void* var = ActiveDocument::Get().PathGet(list_path, typeid(ListType)))
			{
				auto& ref_vector = *static_cast<ListType*>(var);
				if (src_index < ref_vector.size() && dst_index < ref_vector.size())
				{
					auto moved = std::move(ref_vector[src_index]);
					ref_vector.erase(ref_vector.begin() + src_index);
					ref_vector.insert(ref_vector.begin() + dst_index, std::move(moved));
					success = true;
				}
			}

			std::stringstream ss;
			ss << "Redo action " << (success ? "success" : "fail") << ": [path=" << ActiveDocument::Get().PathString(list_path) << ", from_index=" << src_index << ", to_index=" << dst_index << "]";
			Logger::Instance().Log(success ? LogLevel::Success : LogLevel::Error, ss.str());

			return success;
		}

		bool Backward() override
		{
			bool success = false;
			if (void* var = ActiveDocument::Get().PathGet(list_path, typeid(ListType)))
			{
				auto& ref_vector = *static_cast<ListType*>(var);
				if (src_index < ref_vector.size() && dst_index < ref_vector.size())
				{
					auto moved = std::move(ref_vector[dst_index]);
					ref_vector.erase(ref_vector.begin() + dst_index);
					ref_vector.insert(ref_vector.begin() + src_index, std::move(moved));
					success = true;
				}
			}

			std::stringstream ss;
			ss << "Undo action " << (success ? "success" : "fail") << ": [path=" << ActiveDocument::Get().PathString(list_path) << ", from_index=" << dst_index << ", to_index=" << src_index << "]";
			Logger::Instance().Log(success ? LogLevel::Success : LogLevel::Error, ss.str());

			return success;
		}

		size_t EmpiricalSize() const override
		{
			return sizeof(*this);
		}
	};

	template<typename ElementType>
	void ExecuteDynamicListMoveAction(DataPath list_path, size_t src_index, size_t dst_index)
	{
		UndoHistory::ActiveInstance().Execute(std::make_unique<DynamicListMoveAction<ElementType>>(list_path, src_index, dst_index));
	}

	template<typename ElementType>
	struct DynamicListResizeAction : public UndoAction
	{
		using ListType = std::vector<ElementType>;

		DataPathSource list_path;
		size_t initial_size;
		size_t final_size;
		std::vector<ElementType> erased;

		DynamicListResizeAction(DataPath list_path, size_t initial_size, size_t final_size)
			: list_path(list_path), initial_size(initial_size), final_size(final_size)
		{
			// TODO v9.1 use HeapArray for fixed-size runtime smart arrays instead of std::vector, for increased efficiency and size guarantee
			erased.resize(std::max(initial_size, final_size) - std::min(initial_size, final_size));
		}

		bool Forward() override
		{
			bool success = false;
			if (void* var = ActiveDocument::Get().PathGet(list_path, typeid(ListType)))
			{
				auto& ref_vector = *static_cast<ListType*>(var);

				if (initial_size < final_size)
				{
					ref_vector.resize(final_size);
					for (size_t i = initial_size; i < final_size; ++i)
						ref_vector[i] = erased[i - initial_size];
				}
				else if (initial_size > final_size)
				{
					ref_vector.resize(initial_size);
					for (size_t i = final_size; i < initial_size; ++i)
						erased[i - final_size] = ref_vector[i];
					ref_vector.resize(final_size);
				}

				success = true;
			}

			std::stringstream ss;
			ss << "Redo action " << (success ? "success" : "fail") << ": [path=" << ActiveDocument::Get().PathString(list_path) << ", from_size=" << initial_size << ", to_size=" << final_size << "]";
			Logger::Instance().Log(success ? LogLevel::Success : LogLevel::Error, ss.str());

			return success;
		}

		bool Backward() override
		{
			bool success = false;
			if (void* var = ActiveDocument::Get().PathGet(list_path, typeid(ListType)))
			{
				auto& ref_vector = *static_cast<ListType*>(var);

				if (initial_size < final_size)
				{
					ref_vector.resize(final_size);
					for (size_t i = initial_size; i < final_size; ++i)
						erased[i - initial_size] = ref_vector[i];
					ref_vector.resize(initial_size);
				}
				else if (initial_size > final_size)
				{
					ref_vector.resize(initial_size);
					for (size_t i = final_size; i < initial_size; ++i)
						ref_vector[i] = erased[i - final_size];
				}

				success = true;
			}

			std::stringstream ss;
			ss << "Undo action " << (success ? "success" : "fail") << ": [path=" << ActiveDocument::Get().PathString(list_path) << ", from_size=" << final_size << ", to_size=" << initial_size << "]";
			Logger::Instance().Log(success ? LogLevel::Success : LogLevel::Error, ss.str());

			return success;
		}

		size_t EmpiricalSize() const override
		{
			return sizeof(*this) + erased.size() * sizeof(ElementType);
		}
	};

	template<typename ElementType>
	void ExecuteDynamicListResizeAction(DataPath list_path, size_t initial_size, size_t final_size)
	{
		UndoHistory::ActiveInstance().Execute(std::make_unique<DynamicListResizeAction<ElementType>>(list_path, initial_size, final_size));
	}
}
