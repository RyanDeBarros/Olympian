#pragma once

#include "core/algorithms/STLUtils.h"

#include "core/base/Errors.h"

#include "core/context/TickService.h"

#include "external/TOML.h"

#include <imp/auto_tracker.hpp>

#include <memory>
#include <stack>
#include <unordered_set>

namespace oly
{
    template<typename Object>
    struct SmartPoolBase
    {
        using Type = Object;
    };

    template<typename Object>
    using SmartPoolBaseType = typename SmartPoolBase<Object>::Type;

    template<typename T1, typename T2>
    constexpr bool kSharesSmartPoolBase = std::is_same_v<
        SmartPoolBaseType<std::decay_t<T1>>,
        SmartPoolBaseType<std::decay_t<T2>>>;

#define OLY_SMART_POOL_BASE(Class, Base) template<> struct oly::SmartPoolBase<Class> { using Type = Base;\
        static_assert(std::is_base_of_v<Base, Class>, "OLY_SMART_POOL_BASE: Class must derive from base."); };

    template<typename>
    struct SmartReference;

    template<typename>
    struct WeakReference;

    namespace internal
    {
        struct IPool
            : public imp::auto_trackable<IPool>
        {
            virtual void clean() = 0;
            virtual void clear() = 0;
        };

        class PoolBatch final
            : public imp::soft_singleton<PoolBatch>
            , public ITickService
        {
            friend class imp::soft_singleton<PoolBatch>;

        private:
            PoolBatch();

        public:
            void on_tick() override;
            void on_terminate() override;

            void clean();
        };

        struct SmartReferenceLink
        {
            SmartReferenceLink* prev = nullptr;
            SmartReferenceLink* next = nullptr;
        };

        template<typename Object>
        class SmartReferencePool final
            : public imp::soft_singleton<SmartReferencePool<Object>>
            , public IPool
        {
            friend class imp::soft_singleton<SmartReferencePool<Object>>;

            template<typename>
            friend struct SmartReference;

            template<typename>
            friend struct WeakReference;

        private:
            std::vector<std::unique_ptr<Object>> objects_;
            std::stack<size_t> unoccupied_;
            std::unordered_set<size_t> marked_for_deletion_;
            std::vector<SmartReferenceLink*> reference_heads_;
            std::vector<std::pair<void(*)(Object&, void*), void*>> on_delete_callbacks_;

        private:
            ~SmartReferencePool()
            {
                clear();
            }

        public:
            void clean() override;
            void clear() override;

        private:
            size_t init_slot()
            {
                return init_slot(std::make_unique<Object>());
            }

            template<typename T, typename = std::enable_if_t<kSharesSmartPoolBase<Object, T>>>
            size_t init_slot(const T& obj)
            {
                return init_slot(std::make_unique<T>(obj));
            }

            template<typename T, typename = std::enable_if_t<kSharesSmartPoolBase<Object, T>>>
            size_t init_slot(T&& obj)
            {
                return init_slot(std::make_unique<T>(std::move(obj)));
            }

            template<typename T, typename = std::enable_if_t<kSharesSmartPoolBase<Object, T>>>
            size_t init_slot(std::unique_ptr<T>&& obj_ptr)
            {
                if (unoccupied_.empty())
                {
                    objects_.push_back(std::move(obj_ptr));
                    on_delete_callbacks_.push_back({ nullptr, nullptr });
                    reference_heads_.push_back(nullptr);
                    return objects_.size() - 1;
                }
                else
                {
                    size_t next_slot = unoccupied_.top();
                    objects_[next_slot] = std::move(obj_ptr);
                    on_delete_callbacks_[next_slot] = { nullptr, nullptr };
                    unoccupied_.pop();
                    return next_slot;
                }
            }

            void mark_for_deletion(size_t idx)
            {
                marked_for_deletion_.insert(idx);
            }

            void unmark_for_deletion(size_t idx)
            {
                marked_for_deletion_.erase(idx);
            }

            bool is_marked_for_deletion(size_t idx) const
            {
                return marked_for_deletion_.count(idx);
            }

            void increment_references(size_t idx, SmartReferenceLink* link)
            {
                if (reference_heads_[idx])
                {
                    auto* next = reference_heads_[idx]->next;
                    reference_heads_[idx]->next = link;
                    link->prev = reference_heads_[idx];

                    if (next)
                    {
                        link->next = next;
                        next->prev = link;
                    }
                    else
                    {
                        link->next = reference_heads_[idx];
                        reference_heads_[idx]->prev = link;
                    }
                }
                else
                    reference_heads_[idx] = link;
            }

            void decrement_references(size_t idx, SmartReferenceLink* link)
            {
                std::unique_ptr<Object>& object = objects_[idx];
                SmartReferenceLink* prev = link->prev;
                SmartReferenceLink* next = link->next;

                if (next == prev)
                {
                    if (next)
                    {
                        next->prev = nullptr;
                        next->next = nullptr;
                    }
                }
                else
                {
                    if (next)
                        next->prev = prev;
                    if (prev)
                        prev->next = next;
                }

                if (reference_heads_[idx] == link)
                    reference_heads_[idx] = next;

                link->prev = nullptr;
                link->next = nullptr;

                if (!reference_heads_[idx])
                {
                    if (on_delete_callbacks_[idx].first)
                        (*on_delete_callbacks_[idx].first)(*object, on_delete_callbacks_[idx].second);

                    on_delete_callbacks_[idx] = { nullptr, nullptr };

                    marked_for_deletion_.erase(idx);
                    object.reset();
                    unoccupied_.push(idx);
                }
            }

            void replace_references(size_t idx, SmartReferenceLink* existing, SmartReferenceLink* with)
            {
                SmartReferenceLink* prev = existing->prev;
                SmartReferenceLink* next = existing->next;

                existing->prev = nullptr;
                existing->next = nullptr;

                with->prev = prev;
                with->next = next;

                if (prev)
                    prev->next = with;
                if (next)
                    next->prev = with;

                if (reference_heads_[idx] == existing)
                    reference_heads_[idx] = with;
            }
        };

        struct RefInit
        {
        };

        struct RefDefault
        {
        };

        struct RefNull
        {
        };
    }

    constexpr internal::RefInit kRefInit;
    constexpr internal::RefDefault kRefDefault;
    constexpr internal::RefNull kRefNull;

    template<typename Object>
    struct WeakReference
    {
        using PoolBase = SmartPoolBaseType<Object>;

    private:
        size_t pool_idx_ = size_t(-1);
        Object* object_ = nullptr;

    public:
        WeakReference(const SmartReference<Object>& smart_ref);

    public:
        SmartReference<Object> lock() const;

    private:
        static internal::SmartReferencePool<PoolBase>& pool()
        {
            return internal::SmartReferencePool<PoolBase>::instance();
        }
    };

    template<typename Object>
    struct SmartReference
        : private internal::SmartReferenceLink
    {
        using PoolBase = SmartPoolBaseType<Object>;

        friend class internal::SmartReferencePool<PoolBase>;
        friend struct WeakReference<Object>;

    private:
        template<typename T>
        struct IsSmartReferenceWithSharedPoolBase
            : public std::false_type
        {
        };

        template<typename T>
        struct IsSmartReferenceWithSharedPoolBase<SmartReference<T>>
            : public std::bool_constant<kSharesSmartPoolBase<T, Object>>
        {
        };

        template<typename... Args>
        static constexpr bool valid_object_args()
        {
            if constexpr (sizeof...(Args) == 1)
            {
                using T = std::decay_t<std::tuple_element_t<0, std::tuple<Args...>>>;
                return !std::is_same_v<T, nullptr_t>
                    && !std::is_same_v<T, internal::RefInit>
                    && !std::is_same_v<T, internal::RefDefault>
                    && !std::is_same_v<T, internal::RefNull>
                    && !std::is_same_v<T, Object>
                    && !std::is_same_v<T, SmartReference<Object>>
                    && !kSharesSmartPoolBase<T, Object>
                    && !IsSmartReferenceWithSharedPoolBase<T>::value;
            }
            else
                return sizeof...(Args) > 1;
        }

        template<typename... Args>
        using EnableObjectArgs = std::enable_if_t<valid_object_args<Args...>()>;

        template<typename T>
        using SharesPoolBase = std::enable_if_t<kSharesSmartPoolBase<T, Object>>;

        template<typename T>
        using PreventUnconstFrom = std::enable_if_t<!std::is_const_v<T> || std::is_const_v<Object>>;

        template<typename T>
        using PreventUnconstTo = std::enable_if_t<!std::is_const_v<Object> || std::is_const_v<T>>;

    private:
        static inline bool default_valid_ = false;

        size_t pool_idx_ = size_t(-1);

    public:
        SmartReference()
        {
        }

        SmartReference(nullptr_t)
        {
        }

        SmartReference(internal::RefNull)
        {
        }

        SmartReference(internal::RefInit)
            requires (std::is_default_constructible_v<Object>)
        {
            init();
        }

        SmartReference(internal::RefDefault)
            requires (std::is_default_constructible_v<Object>)
        {
            pool_idx_ = default_ref().pool_idx_;
            increment();
        }

        // TODO use make_smart() helper instead of using constructor directly
        explicit SmartReference(const Object& obj)
        {
            init(obj);
        }

        // TODO use make_smart() helper instead of using constructor directly
        explicit SmartReference(Object&& obj)
        {
            init(std::move(obj));
        }

        // TODO use make_smart() helper instead of using constructor directly
        template<typename... Args, typename = EnableObjectArgs<Args...>>
        explicit SmartReference(Args&&... args)
        {
            init(Object(std::forward<Args>(args)...));
        }

        SmartReference(const SmartReference<Object>& o)
            : pool_idx_(o.pool_idx_)
        {
            if (o.valid())
                increment();
        }

        template<typename T, typename = SharesPoolBase<T>, typename = PreventUnconstFrom<T>>
        SmartReference(const SmartReference<T>& o)
            : pool_idx_(o.pool_idx_)
        {
            if (o.valid())
                increment();
        }

        SmartReference(SmartReference<Object>&& o) noexcept
            : pool_idx_(o.pool_idx_)
        {
            if (o.valid())
                replace(o);
        }

        template<typename T, typename = SharesPoolBase<T>, typename = PreventUnconstFrom<T>>
        SmartReference(SmartReference<T>&& o) noexcept
            : pool_idx_(o.pool_idx_)
        {
            if (o.valid())
                replace(o);
        }

        SmartReference<Object>& operator=(const SmartReference<Object>& o)
        {
            if (this != &o)
            {
                if (valid())
                {
                    if (o.valid())
                    {
                        if (pool_idx_ != o.pool_idx_)
                        {
                            decrement();
                            pool_idx_ = o.pool_idx_;
                            increment();
                        }
                    }
                    else
                        decrement();
                }
                else
                {
                    if (o.valid())
                    {
                        pool_idx_ = o.pool_idx_;
                        increment();
                    }
                }
            }
            return *this;
        }

        template<typename T, typename = SharesPoolBase<T>, typename = PreventUnconstFrom<T>>
        SmartReference<Object>& operator=(const SmartReference<T>& o)
        {
            if (this != &o)
            {
                if (valid())
                {
                    if (o.valid())
                    {
                        if (pool_idx_ != o.pool_idx_)
                        {
                            decrement();
                            pool_idx_ = o.pool_idx_;
                            increment();
                        }
                    }
                    else
                        decrement();
                }
                else
                {
                    if (o.valid())
                    {
                        pool_idx_ = o.pool_idx_;
                        increment();
                    }
                }
            }
            return *this;
        }

        SmartReference<Object>& operator=(SmartReference<Object>&& o) noexcept
        {
            if (this != &o)
            {
                if (valid())
                {
                    if (o.valid())
                    {
                        if (pool_idx_ != o.pool_idx_)
                        {
                            decrement();
                            pool_idx_ = o.pool_idx_;
                            replace(o);
                        }
                        else
                            o.decrement();
                    }
                    else
                        decrement();
                }
                else
                {
                    if (o.valid())
                    {
                        pool_idx_ = o.pool_idx_;
                        replace(o);
                    }
                }
            }
            return *this;
        }

        template<typename T, typename = SharesPoolBase<T>, typename = PreventUnconstFrom<T>>
        SmartReference<Object>& operator=(SmartReference<T>&& o) noexcept
        {
            if (this != &o)
            {
                if (valid())
                {
                    if (o.valid())
                    {
                        if (pool_idx_ != o.pool_idx_)
                        {
                            decrement();
                            pool_idx_ = o.pool_idx_;
                            replace(o);
                        }
                        else
                            o.decrement();
                    }
                    else
                        decrement();
                }
                else
                {
                    if (o.valid())
                    {
                        pool_idx_ = o.pool_idx_;
                        replace(o);
                    }
                }
            }
            return *this;
        }

        SmartReference<Object>& operator=(std::nullptr_t)
        {
            invalidate();
            return *this;
        }

        ~SmartReference()
        {
            invalidate();
        }

    public:
        static SmartReference<Object> load(TOMLNode node)
        {
            return SmartReference<Object>(Object::load(node));
        }

        const Object& operator*() const
        {
            if (valid())
                return *static_cast<const Object*>(pool().objects_[pool_idx_].get());
            else
                throw Error(ErrorCode::NullPointer);
        }

        Object& operator*()
        {
            if (valid())
                return *static_cast<Object*>(pool().objects_[pool_idx_].get());
            else
                throw Error(ErrorCode::NullPointer);
        }

        const Object* operator->() const
        {
            if (valid())
                return static_cast<const Object*>(pool().objects_[pool_idx_].get());
            else
                throw Error(ErrorCode::NullPointer);
        }

        Object* operator->()
        {
            if (valid())
                return static_cast<Object*>(pool().objects_[pool_idx_].get());
            else
                throw Error(ErrorCode::NullPointer);
        }

        const PoolBase* base() const
        {
            if (valid())
                return pool().objects_[pool_idx_].get();
            else
                return nullptr;
        }

        PoolBase* base()
        {
            if (valid())
                return pool().objects_[pool_idx_].get();
            else
                return nullptr;
        }

        template<typename T, typename = SharesPoolBase<T>, typename = PreventUnconstTo<T>>
        SmartReference<const T> as() const
        {
            return SmartReference<const T>(*this);
        }

        template<typename T, typename = SharesPoolBase<T>, typename = PreventUnconstTo<T>>
        SmartReference<T> as()
        {
            return SmartReference<T>(*this);
        }

        operator bool() const
        {
            return valid();
        }

        bool valid() const
        {
            return next
                || (pool_idx_ < pool().reference_heads_.size() && pool().reference_heads_[pool_idx_] == link());
        }

        void init()
            requires (std::is_default_constructible_v<Object>)
        {
            if (valid())
                decrement();

            pool_idx_ = pool().init_slot();
            pool().increment_references(pool_idx_, link());
        }

        void init(const Object& obj)
        {
            if (valid())
                decrement();

            pool_idx_ = pool().init_slot(obj);
            increment();
        }

        void init(Object&& obj)
        {
            if (valid())
                decrement();

            pool_idx_ = pool().init_slot(std::move(obj));
            increment();
        }

        template<typename... Args, typename = EnableObjectArgs<Args...>>
        void init(Args&&... args)
        {
            if (valid())
                decrement();

            pool_idx_ = pool().init_slot(Object(std::forward<Args>(args)...));
            increment();
        }

        void init_toml(TOMLNode node)
        {
            init(Object::load(node));
        }

        void clone()
        {
            if (valid())
            {
                Object copy = *pool().objects_[pool_idx_];
                decrement();
                pool_idx_ = pool().init_slot(std::move(copy));
                increment();
            }
        }

        SmartReference<Object> get_clone() const
        {
            SmartReference<Object> cloned;
            if (valid())
            {
                cloned.pool_idx_ = pool().init_slot(*pool().objects_[pool_idx_]);
                cloned.increment();
            }
            return cloned;
        }

        void mark_for_deletion() const
        {
            if (valid())
                pool().mark_for_deletion(pool_idx_);
            else
                throw Error(ErrorCode::NullPointer);
        }

        bool is_marked_for_deletion() const
        {
            if (valid())
                return pool().is_marked_for_deletion(pool_idx_);
            else
                throw Error(ErrorCode::NullPointer);
        }

        void unmark_for_deletion() const
        {
            if (valid())
                pool().unmark_for_deletion(pool_idx_);
            else
                throw Error(ErrorCode::NullPointer);
        }

        void set_on_delete(void(*callback)(Object&, void*), void* usr = nullptr)
        {
            if (valid())
                pool().on_delete_callbacks_[pool_idx_] = { callback, usr };
            else
                throw Error(ErrorCode::NullPointer);
        }

        void invalidate()
        {
            if (valid())
                decrement();
        }

        WeakReference<Object> weak() const
        {
            return WeakReference(*this);
        }

        size_t hash() const
        {
            return std::hash<size_t>{}(valid() ? pool_idx_ : size_t(-1));
        }

        bool operator==(const SmartReference<Object>& o) const
        {
            if (valid())
                return o.valid() && pool_idx_ == o.pool_idx_;
            else
                return !o.valid();
        }

    private:
        static internal::SmartReferencePool<PoolBase>& pool()
        {
            return internal::SmartReferencePool<PoolBase>::instance();
        }

        static SmartReference<Object>& default_ref()
            requires (std::is_default_constructible_v<Object>)
        {
            static SmartReference<Object> def = kRefInit;
            default_valid_ = true;
            return def;
        }

        const SmartReferenceLink* link() const
        {
            return static_cast<const SmartReferenceLink*>(this);
        }

        SmartReferenceLink* link()
        {
            return static_cast<SmartReferenceLink*>(this);
        }

        void increment()
        {
            pool().increment_references(pool_idx_, link());
        }

        void decrement()
        {
            pool().decrement_references(pool_idx_, link());
        }

        template<typename T, typename = SharesPoolBase<T>>
        void replace(SmartReference<T>& o)
        {
            pool().replace_references(pool_idx_, o.link(), link());
        }
    };

    template<typename Object>
    WeakReference<Object>::WeakReference(const SmartReference<Object>& smart_ref)
        : pool_idx_(smart_ref.pool_idx_)
    {
        object_ = pool().objects_[pool_idx_].get();
    }

    template<typename Object>
    SmartReference<Object> WeakReference<Object>::lock() const
    {
        if (internal::SmartReferenceLink* head = pool().reference_heads_[pool_idx_])
        {
            if (pool().objects_[pool_idx_].get() == object_)
                return *static_cast<SmartReference<Object>*>(head);
        }

        throw Error(ErrorCode::BadReference);
    }

    namespace internal
    {
        template<typename Object>
        inline void SmartReferencePool<Object>::clean()
        {
            for (size_t idx : marked_for_deletion_)
            {
                while (reference_heads_[idx])
                {
                    reference_heads_[idx]->prev = nullptr;
                    SmartReferenceLink* next = reference_heads_[idx]->next;
                    reference_heads_[idx]->next = nullptr;
                    reference_heads_[idx] = next;
                }

                objects_[idx] = nullptr;
                on_delete_callbacks_[idx] = { nullptr, nullptr };
            }

            marked_for_deletion_.clear();
        }

        template<typename Object>
        inline void SmartReferencePool<Object>::clear()
        {
            if constexpr (std::is_default_constructible_v<Object>)
            {
                if (SmartReference<Object>::default_valid_)
                {
#ifndef NDEBUG
                    SmartReference<Object>::default_ref().base();
#endif
                    SmartReference<Object>::default_ref().invalidate();
                }
            }

            for (size_t i = 0; i < on_delete_callbacks_.size(); ++i)
            {
                if (on_delete_callbacks_[i].first)
                    (*on_delete_callbacks_[i].first)(*objects_[i], on_delete_callbacks_[i].second);
            }

            on_delete_callbacks_.clear();

            algo::clear_stack(unoccupied_);
            marked_for_deletion_.clear();

            for (SmartReferenceLink* reference_head : reference_heads_)
            {
                while (reference_head)
                {
                    reference_head->prev = nullptr;
                    SmartReferenceLink* next = reference_head->next;
                    reference_head->next = nullptr;
                    reference_head = next;
                }
            }

            reference_heads_.clear();
            objects_.clear();
        }
    }

    namespace internal
    {
        template<typename Object>
        void force_smart_reference_base_instantiation()
        {
            (void)static_cast<typename SmartReference<Object>::PoolBase* (SmartReference<Object>::*)()>(
                &SmartReference<Object>::base);

            (void)static_cast<const typename SmartReference<Object>::PoolBase* (SmartReference<Object>::*)() const>(
                &SmartReference<Object>::base);
        }
    }
}

template<typename T>
struct std::hash<oly::SmartReference<T>>
{
    size_t operator()(const oly::SmartReference<T>& h) const
    {
        return h.hash();
    }
};
