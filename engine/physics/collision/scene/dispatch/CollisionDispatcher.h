#pragma once

#include "physics/collision/scene/dispatch/CollisionController.h"
#include "physics/collision/scene/dispatch/CollisionTree.h"

#include <imp/symmetric_umap.hpp>
#include <imp/bitmask.hpp>

namespace oly::col2d
{
    enum Phase : unsigned char
    {
        Expired = 0b1,
        Started = 0b10,
        Ongoing = 0b100,
        Completed = 0b1000
    };

    IMP_IMPLEMENT_BITMASK(Phase);

    extern Logger::Impl operator<<(Logger::Impl, Phase);

    struct OverlapEventData
    {
        Phase phase;
        const Collider& active_collider;
        const Collider& passive_collider;

        OverlapEventData(
            OverlapResult result,
            const Collider& active_collider,
            const Collider& passive_collider,
            Phase prior);

        OverlapEventData(Phase phase, const Collider& active_collider, const Collider& passive_collider);
        OverlapEventData(const OverlapEventData&);
    };

    extern OverlapEventData invert_event_data(const OverlapEventData& data);

    struct CollisionEventData
    {
        Phase phase;
        float penetration_depth;
        UnitVector2D unit_impulse;
        const Collider& active_collider;
        const Collider& passive_collider;

        CollisionEventData(
            const CollisionResult& result,
            const Collider& active_collider,
            const Collider& passive_collider,
            Phase prior);

        CollisionEventData(
            Phase phase,
            float penetration_depth,
            UnitVector2D unit_impulse,
            const Collider& active_collider,
            const Collider& passive_collider);

        CollisionEventData(const CollisionEventData&);

        glm::vec2 mtv() const;
        OverlapEventData overlap_event() const;
    };

    extern CollisionEventData invert_event_data(const CollisionEventData& data);

    struct ContactEventData
    {
        Phase phase;
        ContactResult::Contact active_contact, passive_contact;
        const Collider& active_collider;
        const Collider& passive_collider;

        ContactEventData(
            const ContactResult& result,
            const Collider& active_collider,
            const Collider& passive_collider,
            Phase prior);

        ContactEventData(
            Phase phase,
            ContactResult::Contact active_contact,
            ContactResult::Contact passive_contact,
            const Collider& active_collider,
            const Collider& passive_collider);

        ContactEventData(const ContactEventData&);

        OverlapEventData overlap_event() const;
        CollisionEventData collision_event() const;
    };

    extern ContactEventData invert_event_data(const ContactEventData& data);

    namespace internal
    {
        class CollisionPhaseTracker
        {
            imp::symmetric_umap<const Collider*, Phase> map_;
            imp::symmetric_umap<const Collider*, Phase>::map_ty lazy_updates_;

        public:
            Phase prior_phase(const Collider& c1, const Collider& c2);
            void lazy_update_phase(const Collider& c1, const Collider& c2, Phase phase);
            void flush();
            void clear();

            void copy_all(const Collider& from, const Collider& to);
            void replace_all(const Collider& at, const Collider& with);
            void erase_all(const Collider& c);
        };

        class CollisionCache
        {
            imp::symmetric_umap<const Collider*, OverlapEventData> overlaps_;
            imp::symmetric_umap<const Collider*, CollisionEventData> collisions_;
            imp::symmetric_umap<const Collider*, ContactEventData> contacts_;

        public:
            void update(const Collider& c1, const Collider& c2, const OverlapEventData& data);
            void update(const Collider& c1, const Collider& c2, const CollisionEventData& data);
            void update(const Collider& c1, const Collider& c2, const ContactEventData& data);

            template<typename T>
            std::optional<T> get(const Collider& c1, const Collider& c2) const
            {
                static_assert(imp::dependent_false_v<T>, "invoked type not supported");
            }

            template<>
            std::optional<OverlapEventData> get<OverlapEventData>(const Collider& c1, const Collider& c2) const
            {
                if (auto data = overlaps_.get(&c1, &c2))
                {
                    if (&data->active_collider == &c1)
                        return *data;
                    else
                        return invert_event_data(*data);
                }
                else if (auto data = collisions_.get(&c1, &c2))
                {
                    if (&data->active_collider == &c1)
                        return data->overlap_event();
                    else
                        return invert_event_data(data->overlap_event());
                }
                else if (auto data = contacts_.get(&c1, &c2))
                {
                    if (&data->active_collider == &c1)
                        return data->overlap_event();
                    else
                        return invert_event_data(data->overlap_event());
                }
                else
                    return std::nullopt;
            }

            template<>
            std::optional<CollisionEventData> get<CollisionEventData>(const Collider& c1, const Collider& c2) const
            {
                if (auto data = collisions_.get(&c1, &c2))
                {
                    if (&data->active_collider == &c1)
                        return *data;
                    else
                        return invert_event_data(*data);
                }
                else if (auto data = contacts_.get(&c1, &c2))
                {
                    if (&data->active_collider == &c1)
                        return data->collision_event();
                    else
                        return invert_event_data(data->collision_event());
                }
                else
                    return std::nullopt;
            }

            template<>
            std::optional<ContactEventData> get<ContactEventData>(const Collider& c1, const Collider& c2) const
            {
                if (auto data = contacts_.get(&c1, &c2))
                {
                    if (&data->active_collider == &c1)
                        return *data;
                    else
                        return invert_event_data(*data);
                }
                else
                    return std::nullopt;
            }

            void clear();

            void copy_all(const Collider& from, const Collider& to);
            void replace_all(const Collider& at, const Collider& with);
            void erase_all(const Collider& c);
        };
    }

    class CollisionDispatcher final
        : public imp::soft_singleton<CollisionDispatcher>
        , public ITickService
    {
        friend class imp::soft_singleton<CollisionDispatcher>;
        friend struct CollisionController;
        friend struct internal::ColliderDispatchHandle;

#define DECLARE_HANDLER_REFS(Type) struct Type##HandlerBase\
        {\
            const CollisionController* controller = nullptr;\
            Type##HandlerBase(CollisionController* controller) : controller(controller) {}\
            Type##HandlerBase(const CollisionController* controller) : controller(controller) {}\
            virtual ~Type##HandlerBase() = default;\
            virtual void invoke(const Type##EventData&) const = 0;\
            virtual const void* raw_handler() const = 0;\
            virtual std::unique_ptr<Type##HandlerBase> clone() const = 0;\
        };\
        struct Type##HandlerRef : Type##HandlerBase\
        {\
            CollisionController::Type##Handler handler = nullptr;\
            Type##HandlerRef(CollisionController& controller, CollisionController::Type##Handler handler)\
                : Type##HandlerBase(&controller), handler(handler) {}\
            void invoke(const Type##EventData& data) const override { (const_cast<CollisionController*>(controller)->*handler)(data); }\
            const void* raw_handler() const override { return reinterpret_cast<const void*>(&handler); }\
            std::unique_ptr<Type##HandlerBase> clone() const override { return std::make_unique<Type##HandlerRef>(*const_cast<CollisionController*>(controller), handler); }\
        };\
        struct Type##ConstHandlerRef : Type##HandlerBase\
        {\
            CollisionController::Type##ConstHandler handler = nullptr;\
            Type##ConstHandlerRef(const CollisionController& controller, CollisionController::Type##ConstHandler handler)\
                : Type##HandlerBase(&controller), handler(handler) {}\
            void invoke(const Type##EventData& data) const override { (controller->*handler)(data); }\
            const void* raw_handler() const override { return reinterpret_cast<const void*>(&handler); }\
            std::unique_ptr<Type##HandlerBase> clone() const override { return std::make_unique<Type##ConstHandlerRef>(*controller, handler); }\
        };\
        struct Type##Hash\
        {\
            using Ptr = std::unique_ptr<Type##HandlerBase>;\
            size_t operator()(const Ptr& a) const { return std::hash<const void*>{}(a->controller) ^ (std::hash<const void*>{}(a->raw_handler()) << 1); }\
            size_t operator()(const std::pair<const Collider*, Ptr>& a) const\
            { return std::hash<const void*>{}(a.first) ^ (Type##Hash{}(a.second) << 1); }\
        };\
        struct Type##Equal\
        {\
            using Ptr = std::unique_ptr<Type##HandlerBase>;\
            bool operator()(const Ptr& a, const Ptr& b) const\
            { return a->controller == b->controller && a->raw_handler() == b->raw_handler(); }\
            bool operator()(const std::pair<const Collider*, Ptr>& a, const std::pair<const Collider*, Ptr>& b) const\
            { return a.first == b.first && Type##Equal{}(a.second, b.second); }\
        };

        DECLARE_HANDLER_REFS(Overlap);
        DECLARE_HANDLER_REFS(Collision);
        DECLARE_HANDLER_REFS(Contact);

#undef DECLARE_HANDLER_REFS

#define HANDLER_MAP(Type) std::unordered_map<const Collider*, std::unordered_set<std::unique_ptr<Type##HandlerBase>, Type##Hash, Type##Equal>>

        HANDLER_MAP(Overlap) overlap_handler_map_;
        HANDLER_MAP(Collision) collision_handler_map_;
        HANDLER_MAP(Contact) contact_handler_map_;

#undef HANDLER_MAP

#define CONTROLLER_LUT(Type) std::unordered_map<const CollisionController*, std::unordered_set<std::pair<const Collider*, std::unique_ptr<Type##HandlerBase>>, Type##Hash, Type##Equal>>

        CONTROLLER_LUT(Overlap) overlap_controller_lut_;
        CONTROLLER_LUT(Collision) collision_controller_lut_;
        CONTROLLER_LUT(Contact) contact_controller_lut_;

#undef CONTROLLER_LUT

        std::vector<CollisionTree> trees_;
        mutable internal::CollisionPhaseTracker phase_tracker_;
        mutable internal::CollisionCache collision_cache_;

        CollisionDispatcher();

    public:
        size_t add_tree(const math::Rect2D bounds, const glm::uvec2 degree = { 2, 2 }, const size_t cell_capacity = 4);
        const CollisionTree& get_tree(size_t i = 0) const;
        void remove_tree(size_t i);
        void clear();

        void unregister_overlap_handlers(const Collider& collider);
        void unregister_collision_handlers(const Collider& collider);
        void unregister_contact_handlers(const Collider& collider);
        void unregister_handlers(const Collider& collider);

        void on_tick() override;

        void emit(const Collider& from);
    };
}
