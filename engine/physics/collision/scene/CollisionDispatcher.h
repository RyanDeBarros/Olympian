#pragma once

#include "physics/collision/scene/CollisionController.h"
#include "core/containers/SymmetricRefMap.h"

#include <unordered_set>

namespace oly::col2d
{
	enum Phase : unsigned char
	{
		EXPIRED   = 0b1,
		STARTED   = 0b10,
		ONGOING   = 0b100,
		COMPLETED = 0b1000
	};

	constexpr Phase operator&(Phase a, Phase b) { return (Phase)((unsigned char)a & (unsigned char)b); }
	constexpr Phase operator|(Phase a, Phase b) { return (Phase)((unsigned char)a | (unsigned char)b); }
	constexpr Phase operator~(Phase a) { return (Phase)(~(unsigned char)a); }

	extern Logger::Impl operator<<(Logger::Impl, Phase);

	struct OverlapEventData
	{
		Phase phase;
		const Collider& active_collider;
		const Collider& passive_collider;

		OverlapEventData(OverlapResult result, const Collider& active_collider, const Collider& passive_collider, Phase prior);

		OverlapEventData(Phase phase, const Collider& active_collider, const Collider& passive_collider)
			: phase(phase), active_collider(active_collider), passive_collider(passive_collider) {}

		OverlapEventData(const OverlapEventData& other)
			: phase(other.phase), active_collider(other.active_collider), passive_collider(other.passive_collider) {}
	};

	inline OverlapEventData invert_event_data(const OverlapEventData& data)
	{
		return OverlapEventData(data.phase, data.passive_collider, data.active_collider);
	}

	struct CollisionEventData
	{
		Phase phase;
		float penetration_depth;
		UnitVector2D unit_impulse;
		const Collider& active_collider;
		const Collider& passive_collider;

		CollisionEventData(const CollisionResult& result, const Collider& active_collider, const Collider& passive_collider, Phase prior);

		CollisionEventData(Phase phase, float penetration_depth, UnitVector2D unit_impulse, const Collider& active_collider, const Collider& passive_collider)
			: phase(phase), penetration_depth(penetration_depth), unit_impulse(unit_impulse), active_collider(active_collider), passive_collider(passive_collider) {}

		CollisionEventData(const CollisionEventData& other)
			: phase(other.phase), penetration_depth(other.penetration_depth), unit_impulse(other.unit_impulse),
			active_collider(other.active_collider), passive_collider(other.passive_collider) {}

		glm::vec2 mtv() const { return (glm::vec2)unit_impulse * penetration_depth; }

		OverlapEventData overlap_event() const
		{
			return OverlapEventData(phase, active_collider, passive_collider);
		}
	};

	inline CollisionEventData invert_event_data(const CollisionEventData& data)
	{
		return CollisionEventData(data.phase, data.penetration_depth, -data.unit_impulse, data.passive_collider, data.active_collider);
	}

	struct ContactEventData
	{
		Phase phase;
		ContactResult::Contact active_contact, passive_contact;
		const Collider& active_collider;
		const Collider& passive_collider;

		ContactEventData(const ContactResult& result, const Collider& active_collider, const Collider& passive_collider, Phase prior);

		ContactEventData(Phase phase, ContactResult::Contact active_contact, ContactResult::Contact passive_contact, const Collider& active_collider, const Collider& passive_collider)
			: phase(phase), active_contact(active_contact), passive_contact(passive_contact), active_collider(active_collider), passive_collider(passive_collider) {}

		ContactEventData(const ContactEventData& other)
			: phase(other.phase), active_contact(other.active_contact), passive_contact(other.passive_contact),
			active_collider(other.active_collider), passive_collider(other.passive_collider) {}

		OverlapEventData overlap_event() const
		{
			return OverlapEventData(phase, active_collider, passive_collider);
		}

		CollisionEventData collision_event() const
		{
			return CollisionEventData(phase, glm::length(active_contact.impulse), UnitVector2D(active_contact.impulse), active_collider, passive_collider);
		}
	};

	inline ContactEventData invert_event_data(const ContactEventData& data)
	{
		return ContactEventData(data.phase, data.passive_contact, data.active_contact, data.passive_collider, data.active_collider);
	}

	namespace internal
	{
		class CollisionPhaseTracker
		{
			SymmetricRefMap<Collider, Phase> map;
			SymmetricRefMap<Collider, Phase>::MapType lazy_updates;

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
			SymmetricRefMap<Collider, OverlapEventData> overlaps;
			SymmetricRefMap<Collider, CollisionEventData> collisions;
			SymmetricRefMap<Collider, ContactEventData> contacts;

		public:
			void update(const Collider& c1, const Collider& c2, const OverlapEventData& data);
			void update(const Collider& c1, const Collider& c2, const CollisionEventData& data);
			void update(const Collider& c1, const Collider& c2, const ContactEventData& data);

			template<typename T>
			std::optional<T> get(const Collider& c1, const Collider& c2) const
			{
				static_assert(deferred_false<T>, "CollisionCache::get<T>() does not support the invoked type.");
			}

			template<>
			std::optional<OverlapEventData> get<OverlapEventData>(const Collider& c1, const Collider& c2) const
			{
				if (auto data = overlaps.get(c1, c2))
				{
					if (&data->active_collider == &c1)
						return *data;
					else
						return invert_event_data(*data);
				}
				else if (auto data = collisions.get(c1, c2))
				{
					if (&data->active_collider == &c1)
						return data->overlap_event();
					else
						return invert_event_data(data->overlap_event());
				}
				else if (auto data = contacts.get(c1, c2))
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
				if (auto data = collisions.get(c1, c2))
				{
					if (&data->active_collider == &c1)
						return *data;
					else
						return invert_event_data(*data);
				}
				else if (auto data = contacts.get(c1, c2))
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
				if (auto data = contacts.get(c1, c2))
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

		class CollisionDispatcher
		{
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
				typedef std::unique_ptr<Type##HandlerBase> Ptr;\
				size_t operator()(const Ptr& a) const { return std::hash<const void*>{}(a->controller) ^ (std::hash<const void*>{}(a->raw_handler()) << 1); }\
				size_t operator()(const std::pair<const Collider*, Ptr>& a) const\
				{ return std::hash<const void*>{}(a.first) ^ (Type##Hash{}(a.second) << 1); }\
			};\
			struct Type##Equal\
			{\
				typedef std::unique_ptr<Type##HandlerBase> Ptr;\
				bool operator()(const Ptr& a, const Ptr& b) const\
				{ return a->controller == b->controller && a->raw_handler() == b->raw_handler(); }\
				bool operator()(const std::pair<const Collider*, Ptr>& a, const std::pair<const Collider*, Ptr>& b) const\
				{ return a.first == b.first && Type##Equal{}(a.second, b.second); }\
			};

			DECLARE_HANDLER_REFS(Overlap);
			DECLARE_HANDLER_REFS(Collision);
			DECLARE_HANDLER_REFS(Contact);

#undef DECLARE_HANDLER_REFS

			friend struct CollisionController;
			friend struct ColliderDispatchHandle;

#define HANDLER_MAP(Type) std::unordered_map<const Collider*, std::unordered_set<std::unique_ptr<Type##HandlerBase>, Type##Hash, Type##Equal>>

			HANDLER_MAP(Overlap) overlap_handler_map;
			HANDLER_MAP(Collision) collision_handler_map;
			HANDLER_MAP(Contact) contact_handler_map;

#undef HANDLER_MAP

#define CONTROLLER_LUT(Type) std::unordered_map<const CollisionController*, std::unordered_set<std::pair<const Collider*, std::unique_ptr<Type##HandlerBase>>, Type##Hash, Type##Equal>>

			CONTROLLER_LUT(Overlap) overlap_controller_lut;
			CONTROLLER_LUT(Collision) collision_controller_lut;
			CONTROLLER_LUT(Contact) contact_controller_lut;

#undef CONTROLLER_LUT

			std::vector<CollisionTree> trees;
			mutable CollisionPhaseTracker phase_tracker;
			mutable CollisionCache collision_cache;

		public:
			CollisionDispatcher() = default;
			CollisionDispatcher(const CollisionDispatcher&) = delete;
			CollisionDispatcher(CollisionDispatcher&&) = delete;

			size_t add_tree(const math::Rect2D bounds, const glm::uvec2 degree = { 2, 2 }, const size_t cell_capacity = 4);
			const CollisionTree& get_tree(size_t i = 0) const { return trees[i]; }
			void remove_tree(size_t i) { trees.erase(trees.begin() + i); }
			void clear();

			void unregister_overlap_handlers(const Collider& collider);
			void unregister_collision_handlers(const Collider& collider);
			void unregister_contact_handlers(const Collider& collider);
			void unregister_handlers(const Collider& collider);

			// call poll() after all collision objects have moved, but before handling events
			void poll();

			void emit(const Collider& from);
		};
	}
}
