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

	public:
		Phase prior_phase(const ConstSoftReference<Collider>& c1, const ConstSoftReference<Collider>& c2);
		void lazy_update_phase(const ConstSoftReference<Collider>& c1, const ConstSoftReference<Collider>& c2, Phase phase);
		void flush();
		void clear();
		void clean();
		void erase(const ConstSoftReference<Collider>& c1, const ConstSoftReference<Collider>& c2);
	};

	namespace internal
	{
		// TODO v3 remove soft references
		class CollisionDispatcher
		{
#define DECLARE_HANDLER_REFS(Type) struct Type##HandlerBase\
			{\
				ConstSoftReference<CollisionController> controller = nullptr;\
				Type##HandlerBase(const SoftReference<CollisionController>& controller) : controller(controller) {}\
				Type##HandlerBase(const ConstSoftReference<CollisionController>& controller) : controller(controller) {}\
				virtual ~Type##HandlerBase() = default;\
				virtual void invoke(const Type##EventData&) const = 0;\
				virtual const void* raw_handler() const = 0;\
				size_t hash() const { return std::hash<const void*>{}(raw_handler()) ^ controller.hash(); }\
			};\
			struct Type##HandlerRef : Type##HandlerBase\
			{\
				CollisionController::Type##Handler handler = nullptr;\
				Type##HandlerRef(const SoftReference<CollisionController>& controller, CollisionController::Type##Handler handler)\
					: Type##HandlerBase(controller), handler(handler) {}\
				virtual void invoke(const Type##EventData& data) const override { (const_cast<CollisionController*>(controller.get())->*handler)(data); }\
				virtual const void* raw_handler() const override { return reinterpret_cast<const void*>(&handler); }\
			};\
			struct Type##ConstHandlerRef : Type##HandlerBase\
			{\
				CollisionController::Type##ConstHandler handler = nullptr;\
				Type##ConstHandlerRef(const ConstSoftReference<CollisionController>& controller, CollisionController::Type##ConstHandler handler)\
					: Type##HandlerBase(controller), handler(handler) {}\
				virtual void invoke(const Type##EventData& data) const override { (controller.get()->*handler)(data); }\
				virtual const void* raw_handler() const override { return reinterpret_cast<const void*>(&handler); }\
			};\
			struct Type##Hash\
			{\
				size_t operator()(const std::unique_ptr<Type##HandlerBase>& ptr) const { return ptr->hash(); }\
			};\
			struct Type##Equal\
			{\
				bool operator()(const std::unique_ptr<Type##HandlerBase>& a, const std::unique_ptr<Type##HandlerBase>& b) const\
				{\
					return a->controller == b->controller && a->raw_handler() == b->raw_handler();\
				}\
			};

			DECLARE_HANDLER_REFS(Overlap);
			DECLARE_HANDLER_REFS(Collision);
			DECLARE_HANDLER_REFS(Contact);

#undef DECLARE_HANDLER_REFS

			friend struct CollisionController;

			mutable std::unordered_map<ConstSoftReference<Collider>, std::unordered_set<std::unique_ptr<OverlapHandlerBase>, OverlapHash, OverlapEqual>> overlap_handlers;
			mutable std::unordered_map<ConstSoftReference<Collider>, std::unordered_set<std::unique_ptr<CollisionHandlerBase>, CollisionHash, CollisionEqual>> collision_handlers;
			mutable std::unordered_map<ConstSoftReference<Collider>, std::unordered_set<std::unique_ptr<ContactHandlerBase>, ContactHash, ContactEqual>> contact_handlers;

			std::vector<CollisionTree> trees;
			mutable CollisionPhaseTracker phase_tracker;

		public:
			CollisionDispatcher() = default;

			size_t add_tree(const math::Rect2D bounds, const glm::uvec2 degree = { 2, 2 }, const size_t cell_capacity = 4);
			const CollisionTree& get_tree(size_t i = 0) const { return trees[i]; }
			void remove_tree(size_t i) { trees.erase(trees.begin() + i); }
			void clear();

			void unregister_overlap_handlers(const ConstSoftReference<Collider>& collider) { overlap_handlers.erase(collider); }
			void unregister_collision_handlers(const ConstSoftReference<Collider>& collider) { collision_handlers.erase(collider); }
			void unregister_contact_handlers(const ConstSoftReference<Collider>& collider) { contact_handlers.erase(collider); }
			void unregister_handlers(const ConstSoftReference<Collider>& collider) { overlap_handlers.erase(collider); collision_handlers.erase(collider); contact_handlers.erase(collider); }

			// call poll() after all collision objects have moved, but before handling events
			void poll() const;
			void clean(); // TODO v3 remove clean() once all soft references are removed

			void emit(const Collider& from);
		};
	}
}
