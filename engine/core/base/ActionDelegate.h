#pragma once

#include "core/base/Errors.h"

#include <functional>
#include <vector>

// TODO move to imp

namespace oly
{
    template<typename... Args>
    class ActionReceiver;

    template<typename... Args>
    class ActionDelegator
    {
        using Receiver = ActionReceiver<Args...>;
        friend class Receiver;

    private:
        std::vector<Receiver*> receivers_;

    public:
        ActionDelegator() = default;

        ActionDelegator(const ActionDelegator<Args...>&) = delete;

        ActionDelegator(ActionDelegator<Args...>&& o) noexcept
            : receivers_(std::move(o.receivers_))
        {
            set_receivers();
        }

        ~ActionDelegator()
        {
            unset_receivers();
        }

        ActionDelegator& operator=(const ActionDelegator<Args...>&) = delete;

        ActionDelegator& operator=(ActionDelegator<Args...>&& o) noexcept
        {
            if (this != &o)
            {
                unset_receivers();
                receivers_ = std::move(o.receivers_);
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
            receivers_.clear();
        }

        size_t receiver_count() const
        {
            return receivers_.size();
        }

        const Receiver& get_receiver(size_t i) const
        {
            return *receivers_[i];
        }

        Receiver& get_receiver(size_t i)
        {
            return *receivers_[i];
        }

        void emit(Args... action) const
        {
            for (Receiver* receiver : receivers_)
                receiver->call(std::forward<Args>(action)...);
        }

    private:
        void set_receivers()
        {
            for (Receiver* receiver : receivers_)
                receiver->delegator_ = this;
        }

        void unset_receivers()
        {
            for (Receiver* receiver : receivers_)
                receiver->delegator_ = nullptr;
        }
    };

    template<typename... Args>
    class ActionReceiver
    {
        using Delegator = ActionDelegator<Args...>;
        friend class Delegator;

    private:
        ActionDelegator<Args...>* delegator_ = nullptr;

    public:
        ActionReceiver() = default;

        ActionReceiver(const ActionReceiver<Args...>& o)
            : delegator_(o.delegator_)
        {
            add_to_delegator();
        }

        ActionReceiver(ActionReceiver<Args...>&& o) noexcept
            : delegator_(o.delegator_)
        {
            o.remove_from_delegator();
            o.delegator_ = nullptr;
            add_to_delegator();
        }

        ~ActionReceiver()
        {
            remove_from_delegator();
        }

        ActionReceiver<Args...>& operator=(const ActionReceiver<Args...>& o) noexcept
        {
            if (this != &o)
            {
                if (delegator_ != o.delegator_)
                {
                    remove_from_delegator();
                    delegator_ = o.delegator_;
                    add_to_delegator();
                }
            }

            return *this;
        }

        ActionReceiver<Args...>& operator=(ActionReceiver<Args...>&& o) noexcept
        {
            if (this != &o)
            {
                if (delegator_ != o.delegator_)
                {
                    remove_from_delegator();
                    delegator_ = o.delegator_;
                    o.remove_from_delegator();
                    o.delegator_ = nullptr;
                    add_to_delegator();
                }
                else
                {
                    o.remove_from_delegator();
                    o.delegator_ = nullptr;
                }
            }

            return *this;
        }

        void connect(Delegator& d)
        {
            if (&d != delegator_)
            {
                if (delegator_)
                    remove_from_delegator();
                delegator_ = &d;
                add_to_delegator();
            }
        }

        void disconnect()
        {
            remove_from_delegator();
        }

        const Delegator& get_delegator() const
        {
            if (delegator_)
                return *delegator_;
            else
                throw Error(ErrorCode::NullPointer);
        }

        Delegator& get_delegator()
        {
            if (delegator_)
                return *delegator_;
            else
                throw Error(ErrorCode::NullPointer);
        }

        virtual void call(Args... action) = 0;

    private:
        void add_to_delegator()
        {
            if (delegator_)
                delegator_->receivers_.push_back(this);
        }

        void remove_from_delegator()
        {
            if (delegator_)
            {
                auto& receivers_ = delegator_->receivers_;
                auto it = std::find(receivers_.begin(), receivers_.end(), this);
                receivers_.erase(it);
            }
        }
    };

    template<typename... Args>
    struct ActionFunctionReceiver
        : public ActionReceiver<Args...>
    {
        using Function = std::function<void(Args...)>;

        Function fn;
        
        ActionFunctionReceiver(Function fn)
            : fn(std::move(fn))
        {
        }

        void call(Args... action) override
        {
            fn(action...);
        }
    };
}
