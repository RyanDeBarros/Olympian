#pragma once

#include "core/Modifiable.h"

#include "gui/DrawResult.h"

#include "desc/DataPath.h"
#include "desc/DynamicListUndoActions.h"

#include "util/Counter.h"

#include <array>
#include <memory>
#include <string>

namespace oly::editor::gui
{
	enum class ListPolicy
	{
		None = 0,
		MinimumOne = 1
	};

	inline ListPolicy operator&(ListPolicy lhs, ListPolicy rhs)
	{
		using T = std::underlying_type_t<ListPolicy>;
		return static_cast<ListPolicy>(static_cast<T>(lhs) & static_cast<T>(rhs));
	}

	inline ListPolicy operator|(ListPolicy lhs, ListPolicy rhs)
	{
		using T = std::underlying_type_t<ListPolicy>;
		return static_cast<ListPolicy>(static_cast<T>(lhs) | static_cast<T>(rhs));
	}

	inline ListPolicy& operator|=(ListPolicy& lhs, ListPolicy rhs)
	{
		lhs = lhs | rhs;
		return lhs;
	}

	enum class ListOpType
	{
		Create,
		Delete,
		Resize,
		Clear,
		Move
	};

	struct ListOp
	{
		ListOpType type;
		bool valid = true;

		size_t index1 = 0;
		size_t index2 = 0;

		static ListOp MakeCreateOp();
		static ListOp MakeDeleteOp(size_t index);
		static ListOp MakeResizeOp(size_t old_size, size_t new_size);
		static ListOp MakeClearOp();
		static ListOp MakeMoveOp(size_t src, size_t dst);

		size_t GetIndex() const;
		size_t GetSrcIndex() const;
		size_t GetDstIndex() const;
		size_t GetOldSize() const;
		size_t GetNewSize() const;

		void Validate(bool valid);
		bool UpdateIndex(ListPolicy policy, size_t& idx) const;
		bool UpdateIndex(ListPolicy policy, Modifiable<size_t>& idx) const;
		bool UpdateIndex(ListPolicy policy, ListOp& op) const;
	};

	struct IListAdapter
	{
		virtual ~IListAdapter() = default;

		virtual size_t Size() const = 0;
		virtual void PushBack() = 0;
		virtual void Erase(size_t index) = 0;
		virtual void Resize(size_t old_size, size_t new_size) = 0;
		virtual void Clear() = 0;
		virtual void Move(size_t src, size_t dst) = 0;
	};

	class ListModel
	{
	private:
		size_t _size = 0;
		std::vector<ListOp> _pending_ops;

	public:
		Modifiable<size_t> active_index = 0;
		ListPolicy policy = ListPolicy::None;

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
		void Apply(const ListOp& op, IListAdapter& adapter);
		void EnforcePolicy(IListAdapter& adapter);

	public:
		void Invoke(const ListOp& op, IListAdapter& adapter);

		DrawResult DrawComboHeader(const char* slot_prefix, const char* create_tooltip, const char* delete_tooltip, const char* clear_tooltip);
		DrawResult DrawComboHeader(std::function<std::string(size_t)> combo_getter, const char* create_tooltip, const char* delete_tooltip, const char* clear_tooltip);
	};

	template<typename T, typename Printer = StandardPrinter<T>>
	struct VectorAdapter : public IListAdapter
	{
		DataPathSource list_path;
		std::vector<T>& v;

		VectorAdapter(DataPath list_path, std::vector<T>& vec) : list_path(list_path), v(vec) {}

		size_t Size() const override
		{
			return v.size();
		}

		void PushBack() override
		{
			ExecuteDynamicListInsertAction<T, Printer>(list_path, v.size());
		}

		void Erase(size_t i) override
		{
			ExecuteDynamicListDeleteAction<T, Printer>(list_path, i);
		}

		void Resize(size_t old_size, size_t new_size) override
		{
			if (old_size != new_size)
				ExecuteDynamicListResizeAction<T>(list_path, old_size, new_size);
		}

		void Clear() override
		{
			if (!v.empty())
				ExecuteDynamicListResizeAction<T>(list_path, v.size(), 0);
		}

		void Move(size_t src, size_t dst) override
		{
			if (src != dst)
				ExecuteDynamicListMoveAction<T>(list_path, src, dst);
		}
	};

	struct ListCallbackAdapter : public IListAdapter
	{
		std::unique_ptr<IListAdapter> primary;
		std::function<void(ListOp)> callback;

		ListCallbackAdapter(std::unique_ptr<IListAdapter>&& primary, std::function<void(ListOp)> callback) : primary(std::move(primary)), callback(std::move(callback)) {}

		size_t Size() const override
		{
			return primary->Size();
		}

		void PushBack() override
		{
			callback(ListOp::MakeCreateOp());
			primary->PushBack();
		}

		void Erase(size_t i) override
		{
			callback(ListOp::MakeDeleteOp(i));
			primary->Erase(i);
		}

		void Resize(size_t old_size, size_t new_size) override
		{
			callback(ListOp::MakeResizeOp(old_size, new_size));
			primary->Resize(old_size, new_size);
		}

		void Clear() override
		{
			callback(ListOp::MakeClearOp());
			primary->Clear();
		}

		void Move(size_t src, size_t dst) override
		{
			callback(ListOp::MakeMoveOp(src, dst));
			primary->Move(src, dst);
		}
	};

	template<typename T, typename Getter, typename Hash = std::hash<T>, typename Equals = std::equal_to<T>>
	std::function<void(ListOp)> MakeCounterCallback(Counter<T, Hash, Equals>& counter, Getter getter)
	{
		return [&counter, getter = std::move(getter)](ListOp op) {
			switch (op.type)
			{
			case ListOpType::Create:
				counter.increment(T{});
				break;

			case ListOpType::Delete:
				counter.decrement(getter(op.GetIndex()));
				break;

			case ListOpType::Resize:
				for (size_t i = op.GetNewSize(); i < op.GetOldSize(); ++i)
					counter.decrement(getter(i));

				if (op.GetOldSize() < op.GetNewSize())
					counter.increment(T{}, op.GetNewSize() - op.GetOldSize());
				break;

			case ListOpType::Clear:
				counter.clear();
				break;
			}
		};
	}
}
