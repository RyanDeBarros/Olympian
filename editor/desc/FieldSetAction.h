#pragma once

#include "core/Printer.h"

#include "documents/IDocument.h"

#include <imp/undo_history.hpp>

#include <sstream>

namespace oly::editor
{
	template<typename T, typename Printer = StandardPrinter<T>>
	struct FieldSetAction : public imp::undo_action
	{
		imtk::datapath path;
		T initial_value;
		T final_value;

		FieldSetAction(imtk::datapath_view path, T initial_value, T final_value) :
			path(path), initial_value(std::move(initial_value)), final_value(std::move(final_value))
		{
		}

		bool forward() override
		{
			bool success = false;
			if (void* var = imtk::active_data_accessor::resolve(path, imp::erase_type<T>()))
			{
				T& ref = *static_cast<T*>(var);
				ref = final_value;
				success = true;
			}

			std::stringstream ss;
			ss << "Redo action " << (success ? "success" : "fail") << ": [path=" << imtk::active_data_accessor::description(path);
			if constexpr (!std::is_void_v<Printer>)
			{
				ss << ", from=";
				Printer{}(ss, initial_value);
				ss << ", to=";
				Printer{}(ss, final_value);
			}
			ss << "]";
			imtk::log(success ? imtk::log_level::success : imtk::log_level::error, ss.str());

			return success;
		}

		bool backward() override
		{
			bool success = false;
			if (void* var = imtk::active_data_accessor::resolve(path, imp::erase_type<T>()))
			{
				T& ref = *static_cast<T*>(var);
				ref = initial_value;
				success = true;
			}

			std::stringstream ss;
			ss << "Undo action " << (success ? "success" : "fail") << ": [path=" << imtk::active_data_accessor::description(path);
			if constexpr (!std::is_void_v<Printer>)
			{
				ss << ", from=";
				Printer{}(ss, final_value);
				ss << ", to=";
				Printer{}(ss, initial_value);
			}
			ss << "]";
			imtk::log(success ? imtk::log_level::success : imtk::log_level::error, ss.str());

			return success;
		}

		size_t empirical_size() const
		{
			return sizeof(*this);
		}
	};

	template<typename T, typename Printer = StandardPrinter<T>>
	void PushFieldSetAction(imtk::datapath_view path, T initial_value, T final_value)
	{
		imp::undo_history::active_instance().push(std::make_unique<FieldSetAction<T, Printer>>(path, std::move(initial_value), std::move(final_value)));
	}

	template<typename Desc, typename Printer = StandardPrinter<Desc>>
	struct DescriptorSetAction : public imp::undo_action
	{
		imtk::datapath path;
		Desc initial_value;
		Desc final_value;

		DescriptorSetAction(imtk::datapath_view path, Desc initial_value, Desc final_value) :
			path(path), initial_value(std::move(initial_value)), final_value(std::move(final_value))
		{
		}

		bool forward() override
		{
			bool success = false;
			if (void* var = imtk::active_data_accessor::resolve(path, imp::erase_type<Desc>()))
			{
				Desc& ref = *static_cast<Desc*>(var);
				ref.copy_data(final_value);
				success = true;
			}

			std::stringstream ss;
			ss << "Redo action " << (success ? "success" : "fail") << ": [path=" << imtk::active_data_accessor::description(path);
			if constexpr (!std::is_void_v<Printer>)
			{
				ss << ", from=";
				Printer{}(ss, initial_value);
				ss << ", to=";
				Printer{}(ss, final_value);
			}
			ss << "]";
			imtk::log(success ? imtk::log_level::success : imtk::log_level::error, ss.str());

			return success;
		}

		bool backward() override
		{
			bool success = false;
			if (void* var = imtk::active_data_accessor::resolve(path, imp::erase_type<Desc>()))
			{
				Desc& ref = *static_cast<Desc*>(var);
				ref.copy_data(initial_value);
				success = true;
			}

			std::stringstream ss;
			ss << "Undo action " << (success ? "success" : "fail") << ": [path=" << imtk::active_data_accessor::description(path);
			if constexpr (!std::is_void_v<Printer>)
			{
				ss << ", from=";
				Printer{}(ss, final_value);
				ss << ", to=";
				Printer{}(ss, initial_value);
			}
			ss << "]";
			imtk::log(success ? imtk::log_level::success : imtk::log_level::error, ss.str());

			return success;
		}

		size_t empirical_size() const
		{
			return sizeof(*this);
		}
	};

	template<typename Desc, typename Printer = StandardPrinter<Desc>>
	void PushDescriptorSetAction(imtk::datapath_view path, Desc initial_value, Desc final_value)
	{
		imp::undo_history::active_instance().push(std::make_unique<DescriptorSetAction<Desc, Printer>>(path, std::move(initial_value), std::move(final_value)));
	}
}
