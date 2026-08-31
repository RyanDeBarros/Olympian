#pragma once

#include <imtk.hpp>

#include <imp/counter.hpp>

namespace oly::editor::gui
{
	struct IListAdapter
	{
		virtual ~IListAdapter() = default;

		virtual size_t Size() const = 0;
		virtual void PushBack() = 0;
		virtual void Erase(size_t index) = 0;
		virtual void Resize(size_t old_size, size_t new_size) = 0;
		virtual void Move(size_t src, size_t dst) = 0;
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

		void PushBack() override
		{
			imtk::desc::execute_vector_insert_action<T, Printer>(v.link.compute_path(), Size());
		}

		void Erase(size_t i) override
		{
			imtk::desc::execute_vector_delete_action<T, Printer>(v.link.compute_path(), i);
		}

		void Resize(size_t old_size, size_t new_size) override
		{
			if (old_size != new_size)
				imtk::desc::execute_vector_resize_action<T>(v.link.compute_path(), old_size, new_size);
		}

		void Move(size_t src, size_t dst) override
		{
			if (src != dst)
				imtk::desc::execute_vector_move_action<T>(v.link.compute_path(), src, dst);
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
		std::function<void(imtk::list_op)> callback;

		ListCallbackAdapter(std::unique_ptr<IListAdapter>&& primary, std::function<void(imtk::list_op)> callback) : primary(std::move(primary)), callback(std::move(callback)) {}

		size_t Size() const override
		{
			return primary->Size();
		}

		void PushBack() override
		{
			callback(imtk::list_op::make_append_op());
			primary->PushBack();
		}

		void Erase(size_t i) override
		{
			callback(imtk::list_op::make_delete_op(i));
			primary->Erase(i);
		}

		void Resize(size_t old_size, size_t new_size) override
		{
			callback(imtk::list_op::make_resize_op(old_size, new_size));
			primary->Resize(old_size, new_size);
		}

		void Move(size_t src, size_t dst) override
		{
			callback(imtk::list_op::make_move_op(src, dst));
			primary->Move(src, dst);
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
