#pragma once

#include "documents/IDocument.h"

#include "util/FixedArray.h"

#include <imp/undo_history.hpp>

#include <sstream>

namespace oly::editor
{
	template<typename ElementType, typename Printer = imtk::standard_printer<ElementType>>
	struct DynamicListDeleteAction : public imp::undo_action
	{
		using ListType = std::vector<ElementType>;

		imtk::datapath list_path;
		size_t delete_index;
		ElementType deleted_element;

		DynamicListDeleteAction(imtk::datapath_view list_path, size_t delete_index, ElementType deleted_element)
			: list_path(list_path), delete_index(delete_index), deleted_element(std::move(deleted_element))
		{
		}

		bool forward() override
		{
			bool success = false;
			if (void* var = imtk::active_data_accessor::resolve(list_path, imp::erase_type<ListType>()))
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
			ss << "Redo action " << (success ? "success" : "fail") << ": [path=" << imtk::active_data_accessor::description(list_path) << ", delete@index=" << delete_index;
			if constexpr (!std::is_void_v<Printer>)
			{
				ss << ", delete@element=";
				Printer{}(ss, deleted_element);
			}
			ss << "]";
			imtk::log(success ? imtk::log_level::success : imtk::log_level::error, ss.str());

			return success;
		}

		bool backward() override
		{
			bool success = false;
			if (void* var = imtk::active_data_accessor::resolve(list_path, imp::erase_type<ListType>()))
			{
				auto& ref_vector = *static_cast<ListType*>(var);
				if (delete_index <= ref_vector.size())
				{
					ref_vector.insert(ref_vector.begin() + delete_index, deleted_element);
					success = true;
				}
			}

			std::stringstream ss;
			ss << "Undo action " << (success ? "success" : "fail") << ": [path=" << imtk::active_data_accessor::description(list_path) << ", re-insert@index=" << delete_index;
			if constexpr (!std::is_void_v<Printer>)
			{
				ss << ", re-insert@element=";
				Printer{}(ss, deleted_element);
			}
			ss << "]";
			imtk::log(success ? imtk::log_level::success : imtk::log_level::error, ss.str());

			return success;
		}

		size_t empirical_size() const override
		{
			return sizeof(*this);
		}
	};

	template<typename ElementType, typename Printer = imtk::standard_printer<ElementType>>
	void ExecuteDynamicListDeleteAction(imtk::datapath_view list_path, size_t delete_index)
	{
		imp::undo_history::active_instance().execute(std::make_unique<DynamicListDeleteAction<ElementType, Printer>>(list_path, delete_index, ElementType{}));
	}

	template<typename ElementType, typename Printer = imtk::standard_printer<ElementType>>
	struct DynamicListInsertAction : public imp::undo_action
	{
		using ListType = std::vector<ElementType>;

		imtk::datapath list_path;
		size_t insert_index;
		ElementType inserted_element;

		DynamicListInsertAction(imtk::datapath_view list_path, size_t insert_index, ElementType inserted_element)
			: list_path(list_path), insert_index(insert_index), inserted_element(std::move(inserted_element))
		{
		}

		bool forward() override
		{
			bool success = false;
			if (void* var = imtk::active_data_accessor::resolve(list_path, imp::erase_type<ListType>()))
			{
				auto& ref_vector = *static_cast<ListType*>(var);
				if (insert_index <= ref_vector.size())
				{
					ref_vector.insert(ref_vector.begin() + insert_index, inserted_element);
					success = true;
				}
			}

			std::stringstream ss;
			ss << "Redo action " << (success ? "success" : "fail") << ": [path=" << imtk::active_data_accessor::description(list_path) << ", insert@index=" << insert_index;
			if constexpr (!std::is_void_v<Printer>)
			{
				ss << ", insert@element=";
				Printer{}(ss, inserted_element);
			}
			ss << "]";
			imtk::log(success ? imtk::log_level::success : imtk::log_level::error, ss.str());

			return success;
		}

		bool backward() override
		{
			bool success = false;
			if (void* var = imtk::active_data_accessor::resolve(list_path, imp::erase_type<ListType>()))
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
			ss << "Undo action " << (success ? "success" : "fail") << ": [path=" << imtk::active_data_accessor::description(list_path) << ", re-delete@index=" << insert_index;
			if constexpr (!std::is_void_v<Printer>)
			{
				ss << ", re-delete@element=";
				Printer{}(ss, inserted_element);
			}
			ss << "]";
			imtk::log(success ? imtk::log_level::success : imtk::log_level::error, ss.str());

			return success;
		}

		size_t empirical_size() const override
		{
			return sizeof(*this);
		}
	};

	template<typename ElementType, typename Printer = imtk::standard_printer<ElementType>>
	void ExecuteDynamicListInsertAction(imtk::datapath_view list_path, size_t insert_index)
	{
		imp::undo_history::active_instance().execute(std::make_unique<DynamicListInsertAction<ElementType, Printer>>(list_path, insert_index, ElementType{}));
	}

	template<typename ElementType>
	struct DynamicListMoveAction : public imp::undo_action
	{
		using ListType = std::vector<ElementType>;

		imtk::datapath list_path;
		size_t src_index;
		size_t dst_index;

		DynamicListMoveAction(imtk::datapath_view list_path, size_t src_index, size_t dst_index)
			: list_path(list_path), src_index(src_index), dst_index(dst_index)
		{
		}

		bool forward() override
		{
			bool success = false;
			if (void* var = imtk::active_data_accessor::resolve(list_path, imp::erase_type<ListType>()))
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
			ss << "Redo action " << (success ? "success" : "fail") << ": [path=" << imtk::active_data_accessor::description(list_path) << ", from_index=" << src_index << ", to_index=" << dst_index << "]";
			imtk::log(success ? imtk::log_level::success : imtk::log_level::error, ss.str());

			return success;
		}

		bool backward() override
		{
			bool success = false;
			if (void* var = imtk::active_data_accessor::resolve(list_path, imp::erase_type<ListType>()))
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
			ss << "Undo action " << (success ? "success" : "fail") << ": [path=" << imtk::active_data_accessor::description(list_path) << ", from_index=" << dst_index << ", to_index=" << src_index << "]";
			imtk::log(success ? imtk::log_level::success : imtk::log_level::error, ss.str());

			return success;
		}

		size_t empirical_size() const override
		{
			return sizeof(*this);
		}
	};

	template<typename ElementType>
	void ExecuteDynamicListMoveAction(imtk::datapath_view list_path, size_t src_index, size_t dst_index)
	{
		imp::undo_history::active_instance().execute(std::make_unique<DynamicListMoveAction<ElementType>>(list_path, src_index, dst_index));
	}

	template<typename ElementType>
	struct DynamicListResizeAction : public imp::undo_action
	{
		using ListType = std::vector<ElementType>;

		imtk::datapath list_path;
		size_t initial_size;
		size_t final_size;
		FixedArray<ElementType> erased;

		DynamicListResizeAction(imtk::datapath_view list_path, size_t initial_size, size_t final_size)
			: list_path(list_path), initial_size(initial_size), final_size(final_size), erased(std::max(initial_size, final_size) - std::min(initial_size, final_size))
		{
		}

		bool forward() override
		{
			bool success = false;
			if (void* var = imtk::active_data_accessor::resolve(list_path, imp::erase_type<ListType>()))
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
			ss << "Redo action " << (success ? "success" : "fail") << ": [path=" << imtk::active_data_accessor::description(list_path) << ", from_size=" << initial_size << ", to_size=" << final_size << "]";
			imtk::log(success ? imtk::log_level::success : imtk::log_level::error, ss.str());

			return success;
		}

		bool backward() override
		{
			bool success = false;
			if (void* var = imtk::active_data_accessor::resolve(list_path, imp::erase_type<ListType>()))
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
			ss << "Undo action " << (success ? "success" : "fail") << ": [path=" << imtk::active_data_accessor::description(list_path) << ", from_size=" << final_size << ", to_size=" << initial_size << "]";
			imtk::log(success ? imtk::log_level::success : imtk::log_level::error, ss.str());

			return success;
		}

		size_t empirical_size() const override
		{
			return sizeof(*this) + erased.length() * sizeof(ElementType);
		}
	};

	template<typename ElementType>
	void ExecuteDynamicListResizeAction(imtk::datapath_view list_path, size_t initial_size, size_t final_size)
	{
		imp::undo_history::active_instance().execute(std::make_unique<DynamicListResizeAction<ElementType>>(list_path, initial_size, final_size));
	}

	template<typename ElementType, typename Printer = imtk::standard_printer<ElementType>>
	struct DynamicVectorDescDeleteAction : public imp::undo_action
	{
		using ListType = imtk::desc::vector<ElementType>;

		imtk::datapath list_path;
		size_t delete_index;
		ElementType deleted_element;

		DynamicVectorDescDeleteAction(imtk::datapath_view list_path, size_t delete_index, ElementType deleted_element)
			: list_path(list_path), delete_index(delete_index), deleted_element(std::move(deleted_element))
		{
		}

		bool forward() override
		{
			bool success = false;
			if (void* var = imtk::active_data_accessor::resolve(list_path, imp::erase_type<ListType>()))
			{
				auto& ref_vector = *static_cast<ListType*>(var);
				if (delete_index < ref_vector.size())
				{
					deleted_element = std::move(ref_vector[delete_index]);
					ref_vector.remove(delete_index);
					success = true;
				}
			}

			std::stringstream ss;
			ss << "Redo action " << (success ? "success" : "fail") << ": [path=" << imtk::active_data_accessor::description(list_path) << ", delete@index=" << delete_index;
			if constexpr (!std::is_void_v<Printer>)
			{
				ss << ", delete@element=";
				Printer{}(ss, deleted_element);
			}
			ss << "]";
			imtk::log(success ? imtk::log_level::success : imtk::log_level::error, ss.str());

			return success;
		}

		bool backward() override
		{
			bool success = false;
			if (void* var = imtk::active_data_accessor::resolve(list_path, imp::erase_type<ListType>()))
			{
				auto& ref_vector = *static_cast<ListType*>(var);
				if (delete_index <= ref_vector.size())
				{
					ref_vector.insert(delete_index, imtk::desc::clone_data(deleted_element));
					success = true;
				}
			}

			std::stringstream ss;
			ss << "Undo action " << (success ? "success" : "fail") << ": [path=" << imtk::active_data_accessor::description(list_path) << ", re-insert@index=" << delete_index;
			if constexpr (!std::is_void_v<Printer>)
			{
				ss << ", re-insert@element=";
				Printer{}(ss, deleted_element);
			}
			ss << "]";
			imtk::log(success ? imtk::log_level::success : imtk::log_level::error, ss.str());

			return success;
		}

		size_t empirical_size() const override
		{
			return sizeof(*this);
		}
	};

	template<typename ElementType, typename Printer = imtk::standard_printer<ElementType>>
	void ExecuteDynamicVectorDescDeleteAction(imtk::datapath_view list_path, size_t delete_index)
	{
		imp::undo_history::active_instance().execute(std::make_unique<DynamicVectorDescDeleteAction<ElementType, Printer>>(list_path, delete_index, ElementType{}));
	}

	template<typename ElementType, typename Printer = imtk::standard_printer<ElementType>>
	struct DynamicVectorDescInsertAction : public imp::undo_action
	{
		using ListType = imtk::desc::vector<ElementType>;

		imtk::datapath list_path;
		size_t insert_index;
		ElementType inserted_element;

		DynamicVectorDescInsertAction(imtk::datapath_view list_path, size_t insert_index, ElementType inserted_element)
			: list_path(list_path), insert_index(insert_index), inserted_element(std::move(inserted_element))
		{
		}

		bool forward() override
		{
			bool success = false;
			if (void* var = imtk::active_data_accessor::resolve(list_path, imp::erase_type<ListType>()))
			{
				auto& ref_vector = *static_cast<ListType*>(var);
				if (insert_index <= ref_vector.size())
				{
					ref_vector.insert(insert_index, imtk::desc::clone_data(inserted_element));
					success = true;
				}
			}

			std::stringstream ss;
			ss << "Redo action " << (success ? "success" : "fail") << ": [path=" << imtk::active_data_accessor::description(list_path) << ", insert@index=" << insert_index;
			if constexpr (!std::is_void_v<Printer>)
			{
				ss << ", insert@element=";
				Printer{}(ss, inserted_element);
			}
			ss << "]";
			imtk::log(success ? imtk::log_level::success : imtk::log_level::error, ss.str());

			return success;
		}

		bool backward() override
		{
			bool success = false;
			if (void* var = imtk::active_data_accessor::resolve(list_path, imp::erase_type<ListType>()))
			{
				auto& ref_vector = *static_cast<ListType*>(var);
				if (insert_index < ref_vector.size())
				{
					inserted_element = std::move(ref_vector[insert_index]);
					ref_vector.remove(insert_index);
					success = true;
				}
			}

			std::stringstream ss;
			ss << "Undo action " << (success ? "success" : "fail") << ": [path=" << imtk::active_data_accessor::description(list_path) << ", re-delete@index=" << insert_index;
			if constexpr (!std::is_void_v<Printer>)
			{
				ss << ", re-delete@element=";
				Printer{}(ss, inserted_element);
			}
			ss << "]";
			imtk::log(success ? imtk::log_level::success : imtk::log_level::error, ss.str());

			return success;
		}

		size_t empirical_size() const override
		{
			return sizeof(*this);
		}
	};

	template<typename ElementType, typename Printer = imtk::standard_printer<ElementType>>
	void ExecuteDynamicVectorDescInsertAction(imtk::datapath_view list_path, size_t insert_index)
	{
		imp::undo_history::active_instance().execute(std::make_unique<DynamicVectorDescInsertAction<ElementType, Printer>>(list_path, insert_index, ElementType{}));
	}

	template<typename ElementType>
	struct DynamicVectorDescMoveAction : public imp::undo_action
	{
		using ListType = imtk::desc::vector<ElementType>;

		imtk::datapath list_path;
		size_t src_index;
		size_t dst_index;

		DynamicVectorDescMoveAction(imtk::datapath_view list_path, size_t src_index, size_t dst_index)
			: list_path(list_path), src_index(src_index), dst_index(dst_index)
		{
		}

		bool forward() override
		{
			bool success = false;
			if (void* var = imtk::active_data_accessor::resolve(list_path, imp::erase_type<ListType>()))
			{
				auto& ref_vector = *static_cast<ListType*>(var);
				if (std::max(src_index, dst_index) < ref_vector.size())
				{
					auto moved = std::move(ref_vector[src_index]);
					ref_vector.remove(src_index);
					ref_vector.insert(dst_index, std::move(moved));
					success = true;
				}
			}

			std::stringstream ss;
			ss << "Redo action " << (success ? "success" : "fail") << ": [path=" << imtk::active_data_accessor::description(list_path) << ", from_index=" << src_index << ", to_index=" << dst_index << "]";
			imtk::log(success ? imtk::log_level::success : imtk::log_level::error, ss.str());

			return success;
		}

		bool backward() override
		{
			bool success = false;
			if (void* var = imtk::active_data_accessor::resolve(list_path, imp::erase_type<ListType>()))
			{
				auto& ref_vector = *static_cast<ListType*>(var);
				if (std::max(src_index, dst_index) < ref_vector.size())
				{
					auto moved = std::move(ref_vector[dst_index]);
					ref_vector.remove(dst_index);
					ref_vector.insert(src_index, std::move(moved));
					success = true;
				}
			}

			std::stringstream ss;
			ss << "Undo action " << (success ? "success" : "fail") << ": [path=" << imtk::active_data_accessor::description(list_path) << ", from_index=" << dst_index << ", to_index=" << src_index << "]";
			imtk::log(success ? imtk::log_level::success : imtk::log_level::error, ss.str());

			return success;
		}

		size_t empirical_size() const override
		{
			return sizeof(*this);
		}
	};

	template<typename ElementType>
	void ExecuteDynamicVectorDescMoveAction(imtk::datapath_view list_path, size_t src_index, size_t dst_index)
	{
		imp::undo_history::active_instance().execute(std::make_unique<DynamicVectorDescMoveAction<ElementType>>(list_path, src_index, dst_index));
	}

	template<typename ElementType>
	struct DynamicVectorDescResizeAction : public imp::undo_action
	{
		using ListType = imtk::desc::vector<ElementType>;

		imtk::datapath list_path;
		size_t initial_size;
		size_t final_size;
		FixedArray<ElementType> erased;

		DynamicVectorDescResizeAction(imtk::datapath_view list_path, size_t initial_size, size_t final_size)
			: list_path(list_path), initial_size(initial_size), final_size(final_size), erased(std::max(initial_size, final_size) - std::min(initial_size, final_size))
		{
		}

		bool forward() override
		{
			bool success = false;
			if (void* var = imtk::active_data_accessor::resolve(list_path, imp::erase_type<ListType>()))
			{
				auto& ref_vector = *static_cast<ListType*>(var);

				if (initial_size < final_size)
				{
					ref_vector.resize(final_size);
					for (size_t i = initial_size; i < final_size; ++i)
						ref_vector[i].copy_data(erased[i - initial_size]);
				}
				else if (initial_size > final_size)
				{
					ref_vector.resize(initial_size);
					for (size_t i = final_size; i < initial_size; ++i)
						erased[i - final_size].copy_data(ref_vector[i]);
					ref_vector.resize(final_size);
				}

				success = true;
			}

			std::stringstream ss;
			ss << "Redo action " << (success ? "success" : "fail") << ": [path=" << imtk::active_data_accessor::description(list_path) << ", from_size=" << initial_size << ", to_size=" << final_size << "]";
			imtk::log(success ? imtk::log_level::success : imtk::log_level::error, ss.str());

			return success;
		}

		bool backward() override
		{
			bool success = false;
			if (void* var = imtk::active_data_accessor::resolve(list_path, imp::erase_type<ListType>()))
			{
				auto& ref_vector = *static_cast<ListType*>(var);

				if (initial_size < final_size)
				{
					ref_vector.resize(final_size);
					for (size_t i = initial_size; i < final_size; ++i)
						erased[i - initial_size].copy_data(ref_vector[i]);
					ref_vector.resize(initial_size);
				}
				else if (initial_size > final_size)
				{
					ref_vector.resize(initial_size);
					for (size_t i = final_size; i < initial_size; ++i)
						ref_vector[i].copy_data(erased[i - final_size]);
				}

				success = true;
			}

			std::stringstream ss;
			ss << "Undo action " << (success ? "success" : "fail") << ": [path=" << imtk::active_data_accessor::description(list_path) << ", from_size=" << final_size << ", to_size=" << initial_size << "]";
			imtk::log(success ? imtk::log_level::success : imtk::log_level::error, ss.str());

			return success;
		}

		size_t empirical_size() const override
		{
			return sizeof(*this) + erased.length() * sizeof(ElementType);
		}
	};

	// TODO v9.3 use imtk::desc::... and imtk::field::... to differentiate between utilities, instead of this convoluted name

	template<typename ElementType>
	void ExecuteDynamicVectorDescResizeAction(imtk::datapath_view list_path, size_t initial_size, size_t final_size)
	{
		imp::undo_history::active_instance().execute(std::make_unique<DynamicVectorDescResizeAction<ElementType>>(list_path, initial_size, final_size));
	}
}
