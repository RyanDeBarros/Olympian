#pragma once

#include "core/base/Errors.h"

#include <vector>
#include <functional>

namespace oly
{
	template<typename... Args>
	class ActionReceiver;

	template<typename... Args>
	class ActionDelegator
	{
		using Receiver = ActionReceiver<Args...>;
		friend class Receiver;

		std::vector<Receiver*> receivers;

	public:
		ActionDelegator() = default;
		
		ActionDelegator(const ActionDelegator<Args...>&) = delete; // TODO v4 delete transformer copy constructor
		
		ActionDelegator(ActionDelegator<Args...>&& other) noexcept
			: receivers(std::move(other))
		{
			set_receivers();
		}

		~ActionDelegator()
		{
			unset_receivers();
		}

		ActionDelegator<Args...>& operator=(ActionDelegator<Args...>&& other) noexcept
		{
			if (this != &other)
			{
				unset_receivers();
				receivers = std::move(other.receivers);
				set_receivers();
			}
			return *this;
		}

		void connect(Receiver& receiver)
		{
			receiver.connect(*this);
		}

		void disconnect_all()
		{
			unset_receivers();
			receivers.clear();
		}

		size_t receiver_count() const
		{
			return receivers.size();
		}

		const Receiver& get_receiver(size_t i) const
		{
			return *receivers[i];
		}

		Receiver& get_receiver(size_t i)
		{
			return *receivers[i];
		}

		void emit(Args... action) const
		{
			for (Receiver* receiver : receivers)
				receiver->call(std::forward<Args>(action)...);
		}

	private:
		void set_receivers()
		{
			for (Receiver* receiver : receivers)
				receiver->delegator = this;
		}

		void unset_receivers()
		{
			for (Receiver* receiver : receivers)
				receiver->delegator = nullptr;
		}
	};

	template<typename... Args>
	class ActionReceiver
	{
		using Delegator = ActionDelegator<Args...>;
		friend class Delegator;

		ActionDelegator<Args...>* delegator = nullptr;

	public:
		ActionReceiver() = default;

		ActionReceiver(const ActionReceiver<Args...>& other)
			: delegator(other.delegator)
		{
			add_to_delegator();
		}

		ActionReceiver(ActionReceiver<Args...>&& other) noexcept
			: delegator(other.delegator)
		{
			other.remove_from_delegator();
			other.delegator = nullptr;
			add_to_delegator();
		}

		~ActionReceiver()
		{
			remove_from_delegator();
		}

		ActionReceiver<Args...>& operator=(const ActionReceiver<Args...>& other) noexcept
		{
			if (this != &other)
			{
				if (delegator != other.delegator)
				{
					remove_from_delegator();
					delegator = other.delegator;
					add_to_delegator();
				}
			}
			return *this;
		}

		ActionReceiver<Args...>& operator=(ActionReceiver<Args...>&& other) noexcept
		{
			if (this != &other)
			{
				if (delegator != other.delegator)
				{
					remove_from_delegator();
					delegator = other.delegator;
					other.remove_from_delegator();
					other.delegator = nullptr;
					add_to_delegator();
				}
				else
				{
					other.remove_from_delegator();
					other.delegator = nullptr;
				}
			}
			return *this;
		}

		void connect(Delegator& d)
		{
			if (&d != delegator)
			{
				if (delegator)
					remove_from_delegator();
				delegator = &d;
				add_to_delegator();
			}
		}

		void disconnect()
		{
			remove_from_delegator();
		}

		const Delegator& get_delegator() const
		{
			if (delegator)
				return *delegator;
			else
				throw Error(ErrorCode::NULL_POINTER);
		}

		Delegator& get_delegator()
		{
			if (delegator)
				return *delegator;
			else
				throw Error(ErrorCode::NULL_POINTER);
		}

		virtual void call(Args... action) = 0;

	private:
		void add_to_delegator()
		{
			if (delegator)
				delegator->receivers.push_back(this);
		}

		void remove_from_delegator()
		{
			if (delegator)
			{
				auto& receivers = delegator->receivers;
				auto it = std::find(receivers.begin(), receivers.end(), this);
				receivers.erase(it);
			}
		}
	};

	template<typename... Args>
	struct ActionFunctionReceiver : public ActionReceiver<Args...>
	{
		using Function = std::function<void(Args...)>;

		Function fn;

		ActionFunctionReceiver(const Function& fn) : fn(fn) {}
		ActionFunctionReceiver(Function&& fn) : fn(std::move(fn)) {}

		void call(Args... action) override
		{
			fn(action...);
		}
	};
}
