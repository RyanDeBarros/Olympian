#pragma once

#include "physics/collision/scene/CollisionController.h"

#include <unordered_set>

namespace oly::col2d
{
	enum Phase : unsigned char
	{
		EXPIRED = 0,
		STARTED = 0b1,
		ONGOING = 0b10,
		COMPLETED = 0b100
	};

	constexpr Phase operator&(Phase a, Phase b) { return (Phase)((unsigned char)a & (unsigned char)b); }
	constexpr Phase operator|(Phase a, Phase b) { return (Phase)((unsigned char)a | (unsigned char)b); }
	constexpr Phase operator~(Phase a) { return (Phase)(~(unsigned char)a); }

	extern Logger& operator<<(Logger&, Phase);

	struct OverlapEventData
	{
		Phase phase;
		ConstSoftReference<Collider> active_collider, passive_collider;

		OverlapEventData(OverlapResult result, const ConstSoftReference<Collider>& active_collider, const ConstSoftReference<Collider>& passive_collider, Phase prior);

		OverlapEventData& invert() { std::swap(active_collider, passive_collider); return *this; }
	};

	struct CollisionEventData
	{
		Phase phase;
		float penetration_depth;
		UnitVector2D unit_impulse;
		ConstSoftReference<Collider> active_collider, passive_collider;

		CollisionEventData(const CollisionResult& result, const ConstSoftReference<Collider>& active_collider, const ConstSoftReference<Collider>& passive_collider, Phase prior);

		CollisionEventData& invert() { unit_impulse = -unit_impulse; std::swap(active_collider, passive_collider); return *this; }
		glm::vec2 mtv() const { return (glm::vec2)unit_impulse * penetration_depth; }
	};

	struct ContactEventData
	{
		Phase phase;
		ContactResult::Contact active_contact, passive_contact;
		ConstSoftReference<Collider> active_collider, passive_collider;

		ContactEventData(const ContactResult& result, const ConstSoftReference<Collider>& active_collider, const ConstSoftReference<Collider>& passive_collider, Phase prior);

		ContactEventData& invert() { std::swap(active_contact, passive_contact); std::swap(active_collider, passive_collider); return *this; }
	};

	namespace internal
	{
		class CollisionPhaseTracker
		{
			struct ColliderUnorderedPair
			{
				ConstSoftReference<Collider> c1;
				ConstSoftReference<Collider> c2;

				bool operator==(const ColliderUnorderedPair& other) const { return (c1 == other.c1 && c2 == other.c2) || (c1 == other.c2 && c2 == other.c1); }
			};

			struct ColliderUnorderedPairHash
			{
				size_t operator()(const ColliderUnorderedPair& pair) const { return pair.c1.hash() ^ pair.c2.hash(); }
			};

			std::unordered_map<ColliderUnorderedPair, Phase, ColliderUnorderedPairHash> map;
			std::unordered_map<ColliderUnorderedPair, Phase, ColliderUnorderedPairHash> lazy_updates;
			std::unordered_map<ConstSoftReference<Collider>, std::unordered_set<ConstSoftReference<Collider>>> lut;

		public:
			Phase prior_phase(const ConstSoftReference<Collider>& c1, const ConstSoftReference<Collider>& c2);
			void lazy_update_phase(const ConstSoftReference<Collider>& c1, const ConstSoftReference<Collider>& c2, Phase phase);
			void flush();
			void clear();
			void clean(); // TODO v3 remove when no more soft references

			void copy_all(const ConstSoftReference<Collider>& from, const ConstSoftReference<Collider>& to);
			void replace_all(const ConstSoftReference<Collider>& at, const ConstSoftReference<Collider>& with);
			void erase_all(const ConstSoftReference<Collider>& c);
		};

		// TODO v3 remove soft references
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
				size_t operator()(const std::pair<ConstSoftReference<Collider>, Ptr>& a) const\
				{ return a.first.hash() ^ (Type##Hash{}(a.second) << 1); }\
			};\
			struct Type##Equal\
			{\
				typedef std::unique_ptr<Type##HandlerBase> Ptr;\
				bool operator()(const Ptr& a, const Ptr& b) const\
				{ return a->controller == b->controller && a->raw_handler() == b->raw_handler(); }\
				bool operator()(const std::pair<ConstSoftReference<Collider>, Ptr>& a, const std::pair<ConstSoftReference<Collider>, Ptr>& b) const\
				{ return a.first == b.first && Type##Equal{}(a.second, b.second); }\
			};

			DECLARE_HANDLER_REFS(Overlap);
			DECLARE_HANDLER_REFS(Collision);
			DECLARE_HANDLER_REFS(Contact);

#undef DECLARE_HANDLER_REFS

			friend struct CollisionController;
			friend struct ColliderDispatchHandle;

#define HANDLER_MAP(Type) std::unordered_map<ConstSoftReference<Collider>, std::unordered_set<std::unique_ptr<Type##HandlerBase>, Type##Hash, Type##Equal>>

			HANDLER_MAP(Overlap) overlap_handler_map;
			HANDLER_MAP(Collision) collision_handler_map;
			HANDLER_MAP(Contact) contact_handler_map;

#undef HANDLER_MAP

#define CONTROLLER_LUT(Type) std::unordered_map<const CollisionController*, std::unordered_set<std::pair<ConstSoftReference<Collider>, std::unique_ptr<Type##HandlerBase>>, Type##Hash, Type##Equal>>

			CONTROLLER_LUT(Overlap) overlap_controller_lut;
			CONTROLLER_LUT(Collision) collision_controller_lut;
			CONTROLLER_LUT(Contact) contact_controller_lut;

#undef CONTROLLER_LUT

			std::vector<CollisionTree> trees;
			mutable CollisionPhaseTracker phase_tracker;

		public:
			CollisionDispatcher() = default;
			CollisionDispatcher(const CollisionDispatcher&) = delete;
			CollisionDispatcher(CollisionDispatcher&&) = delete;

			size_t add_tree(const math::Rect2D bounds, const glm::uvec2 degree = { 2, 2 }, const size_t cell_capacity = 4);
			const CollisionTree& get_tree(size_t i = 0) const { return trees[i]; }
			void remove_tree(size_t i) { trees.erase(trees.begin() + i); }
			void clear();

			void unregister_overlap_handlers(const ConstSoftReference<Collider>& collider);
			void unregister_collision_handlers(const ConstSoftReference<Collider>& collider);
			void unregister_contact_handlers(const ConstSoftReference<Collider>& collider);
			void unregister_handlers(const ConstSoftReference<Collider>& collider);

			// call poll() after all collision objects have moved, but before handling events
			void poll();
			void clean(); // TODO v3 remove clean() once all soft references are removed

			void emit(const Collider& from);
		};
	}
}
