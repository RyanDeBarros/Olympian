#pragma once

#include <imtk.hpp>

#include <imp/counter.hpp>

namespace oly::editor::gui
{
	class ListModel
	{
	public:
		// TODO v9.3 obviously collapse this to just imtk::list_model
		imtk::list_model model;

		struct ComboHeader
		{
			const char* prompt;
			const char* create_tooltip;
			const char* delete_tooltip;
			const char* clear_tooltip;
		};

		imtk::item_result DrawComboHeader(const ComboHeader& header, const char* slot_prefix);
		imtk::item_result DrawComboHeader(const ComboHeader& header, std::function<std::string(size_t)> combo_getter);
	};

	template<typename T, typename Printer = imtk::standard_printer<T>>
	struct VectorAdapter : public imtk::list_adapter
	{
		const imtk::desc::vector<T>& v;

		VectorAdapter(const imtk::desc::vector<T>& vec) : v(vec) {}

		size_t size() const override
		{
			return v.size();
		}

		void apply(const imtk::list_op& op) override
		{
			op.execute_desc_action<T, Printer>(v.link.compute_path());
		}
	};

	template<typename T>
	std::unique_ptr<VectorAdapter<T, imtk::standard_printer<T>>> MakeVectorAdapter(const imtk::desc::vector<T>& vector)
	{
		return std::make_unique<VectorAdapter<T, imtk::standard_printer<T>>>(vector);
	}

	template<typename Printer, typename T>
	std::unique_ptr<VectorAdapter<T, Printer>> MakeVectorAdapter(const imtk::desc::vector<T>& vector)
	{
		return std::make_unique<VectorAdapter<T, Printer>>(vector);
	}

	struct ListCallbackAdapter : public imtk::list_adapter
	{
		std::unique_ptr<imtk::list_adapter> primary;
		std::function<void(const imtk::list_op&)> callback;

		ListCallbackAdapter(std::unique_ptr<imtk::list_adapter>&& primary, std::function<void(const imtk::list_op&)> callback) : primary(std::move(primary)), callback(std::move(callback)) {}

		size_t size() const override
		{
			return primary->size();
		}

		void apply(const imtk::list_op& op) override
		{
			callback(op);
			primary->apply(op);
		}
	};

	template<typename T, typename Getter, typename Hash = std::hash<T>, typename Equals = std::equal_to<T>>
	std::function<void(imtk::list_op)> MakeCounterCallback(imp::counter<T, Hash, Equals>& counter, Getter getter)
	{
		return [&counter, getter = std::move(getter)](imtk::list_op op) {
			switch (op.type())
			{
			case imtk::list_op_type::append_:
				counter.increment(T{});
				break;

			case imtk::list_op_type::delete_:
				counter.decrement(getter(op.get_index()));
				break;

			case imtk::list_op_type::resize_:
				if (op.get_new_size() == 0)
					counter.clear();
				else
				{
					for (size_t i = op.get_new_size(); i < op.get_old_size(); ++i)
						counter.decrement(getter(i));

					if (op.get_old_size() < op.get_new_size())
						counter.increment(T{}, op.get_new_size() - op.get_old_size());
				}
				break;
			}
		};
	}
}
