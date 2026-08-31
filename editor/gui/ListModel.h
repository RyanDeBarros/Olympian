#pragma once

#include <imtk.hpp>

#include <imp/counter.hpp>

namespace oly::editor::gui
{
	struct IListAdapter
	{
		virtual ~IListAdapter() = default;

		virtual size_t Size() const = 0;
		virtual void Apply(const imtk::list_op& op) = 0;
	};

	class ListModel
	{
	private:
		size_t _size = 0;
		std::vector<imtk::list_op> _pending_ops;

	public:
		imp::modifiable<size_t> active_index = 0;
		imtk::list_policy policy = imtk::list_policy::none;

		void Init(IListAdapter& adapter);
		void Update(IListAdapter& adapter);

		size_t Size() const;

	private:
		void Clamp();
		void SetLast();

	public:
		void DeferCreate();
		void DeferDelete();
		void DeferResize(size_t new_size);
		void DeferClear();

		bool ConsumeOps(IListAdapter& adapter);

	private:
		void Apply(const imtk::list_op& op, IListAdapter& adapter);
		void EnforcePolicy(IListAdapter& adapter);

	public:
		void Invoke(const imtk::list_op& op, IListAdapter& adapter);

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
	struct VectorAdapter : public IListAdapter
	{
		const imtk::desc::vector<T>& v;

		VectorAdapter(const imtk::desc::vector<T>& vec) : v(vec) {}

		size_t Size() const override
		{
			return v.size();
		}

		void Apply(const imtk::list_op& op) override
		{
			op.execute_desc_action<T, Printer>(v.link.compute_path());
		}
	};

	template<typename T>
	std::unique_ptr<IListAdapter> MakeVectorAdapter(const imtk::desc::vector<T>& vector)
	{
		return std::make_unique<VectorAdapter<T, imtk::standard_printer<T>>>(vector);
	}

	template<typename Printer, typename T>
	std::unique_ptr<IListAdapter> MakeVectorAdapter(const imtk::desc::vector<T>& vector)
	{
		return std::make_unique<VectorAdapter<T, Printer>>(vector);
	}

	struct ListCallbackAdapter : public IListAdapter
	{
		std::unique_ptr<IListAdapter> primary;
		std::function<void(const imtk::list_op&)> callback;

		ListCallbackAdapter(std::unique_ptr<IListAdapter>&& primary, std::function<void(const imtk::list_op&)> callback) : primary(std::move(primary)), callback(std::move(callback)) {}

		size_t Size() const override
		{
			return primary->Size();
		}

		void Apply(const imtk::list_op& op) override
		{
			callback(op);
			primary->Apply(op);
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
