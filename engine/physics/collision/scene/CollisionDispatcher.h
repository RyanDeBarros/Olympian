#pragma once

#include "physics/collision/scene/Collider.h"

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

	struct CollisionController
	{
		OLY_SOFT_REFERENCE_BASE_DECLARATION(CollisionController);

	public:
		virtual ~CollisionController() = default;

		using OverlapHandler = void(CollisionController::*)(const OverlapEventData&);
		using OverlapConstHandler = void(CollisionController::*)(const OverlapEventData&) const;
		using CollisionHandler = void(CollisionController::*)(const CollisionEventData&);
		using CollisionConstHandler = void(CollisionController::*)(const CollisionEventData&) const;
		using ContactHandler = void(CollisionController::*)(const ContactEventData&);
		using ContactConstHandler = void(CollisionController::*)(const ContactEventData&) const;
	};

#define OLY_COLLISION_CONTROLLER_HEADER(Class)\
	OLY_SOFT_REFERENCE_PUBLIC(Class)

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
			struct OverlapHandlerBase
			{
				ConstSoftReference<CollisionController> controller = nullptr;

				OverlapHandlerBase(const SoftReference<CollisionController>& controller) : controller(controller) {}
				OverlapHandlerBase(const ConstSoftReference<CollisionController>& controller) : controller(controller) {}
				virtual ~OverlapHandlerBase() = default;

				virtual void invoke(const OverlapEventData&) const = 0;
			};

			struct OverlapHandlerRef : OverlapHandlerBase
			{
				CollisionController::OverlapHandler handler = nullptr;

				OverlapHandlerRef(const SoftReference<CollisionController>& controller, CollisionController::OverlapHandler handler) : OverlapHandlerBase(controller), handler(handler) {}

				virtual void invoke(const OverlapEventData& data) const override { (const_cast<CollisionController*>(controller.get())->*handler)(data); }
			};

			struct OverlapConstHandlerRef : OverlapHandlerBase
			{
				CollisionController::OverlapConstHandler handler = nullptr;

				OverlapConstHandlerRef(const ConstSoftReference<CollisionController>& controller, CollisionController::OverlapConstHandler handler) : OverlapHandlerBase(controller), handler(handler) {}

				virtual void invoke(const OverlapEventData& data) const override { (controller.get()->*handler)(data); }
			};

			struct CollisionHandlerBase
			{
				ConstSoftReference<CollisionController> controller = nullptr;

				CollisionHandlerBase(const SoftReference<CollisionController>& controller) : controller(controller) {}
				CollisionHandlerBase(const ConstSoftReference<CollisionController>& controller) : controller(controller) {}
				virtual ~CollisionHandlerBase() = default;

				virtual void invoke(const CollisionEventData&) const = 0;
			};

			struct CollisionHandlerRef : CollisionHandlerBase
			{
				CollisionController::CollisionHandler handler = nullptr;

				CollisionHandlerRef(const SoftReference<CollisionController>& controller, CollisionController::CollisionHandler handler) : CollisionHandlerBase(controller), handler(handler) {}

				virtual void invoke(const CollisionEventData& data) const override { (const_cast<CollisionController*>(controller.get())->*handler)(data); }
			};

			struct CollisionConstHandlerRef : CollisionHandlerBase
			{
				CollisionController::CollisionConstHandler handler = nullptr;

				CollisionConstHandlerRef(const ConstSoftReference<CollisionController>& controller, CollisionController::CollisionConstHandler handler) : CollisionHandlerBase(controller), handler(handler) {}

				virtual void invoke(const CollisionEventData& data) const override { (controller.get()->*handler)(data); }
			};

			struct ContactHandlerBase
			{
				ConstSoftReference<CollisionController> controller = nullptr;

				ContactHandlerBase(const SoftReference<CollisionController>& controller) : controller(controller) {}
				ContactHandlerBase(const ConstSoftReference<CollisionController>& controller) : controller(controller) {}
				virtual ~ContactHandlerBase() = default;

				virtual void invoke(const ContactEventData&) const = 0;
			};

			struct ContactHandlerRef : ContactHandlerBase
			{
				CollisionController::ContactHandler handler = nullptr;

				ContactHandlerRef(const SoftReference<CollisionController>& controller, CollisionController::ContactHandler handler) : ContactHandlerBase(controller), handler(handler) {}

				virtual void invoke(const ContactEventData& data) const override { (const_cast<CollisionController*>(controller.get())->*handler)(data); }
			};

			struct ContactConstHandlerRef : ContactHandlerBase
			{
				CollisionController::ContactConstHandler handler = nullptr;

				ContactConstHandlerRef(const ConstSoftReference<CollisionController>& controller, CollisionController::ContactConstHandler handler) : ContactHandlerBase(controller), handler(handler) {}

				virtual void invoke(const ContactEventData& data) const override { (controller.get()->*handler)(data); }
			};

			template<typename T>
			struct HandlerRefHash
			{
				size_t operator()(const T& v) const
				{
					return std::visit([](const auto& v) {
						return std::hash<const void*>{}(reinterpret_cast<const void*>(&v.handler)) ^ std::hash<decltype(v.controller)>{}(v.controller);
						}, v);
				}
			};

			mutable std::unordered_map<ConstSoftReference<Collider>, std::unordered_set<std::unique_ptr<OverlapHandlerBase>>> overlap_handlers;
			mutable std::unordered_map<ConstSoftReference<Collider>, std::unordered_set<std::unique_ptr<CollisionHandlerBase>>> collision_handlers;
			mutable std::unordered_map<ConstSoftReference<Collider>, std::unordered_set<std::unique_ptr<ContactHandlerBase>>> contact_handlers;

			std::vector<CollisionTree> trees;
			mutable CollisionPhaseTracker phase_tracker;

		public:
			CollisionDispatcher() = default;

			void add_tree(const math::Rect2D bounds, const glm::uvec2 degree = { 2, 2 }, const size_t cell_capacity = 4)
			{
				trees.emplace_back(bounds, degree, cell_capacity);
			}

			void clear() { trees.clear(); overlap_handlers.clear(); collision_handlers.clear(); contact_handlers.clear(); phase_tracker.clear(); }

			const CollisionTree& get_tree(size_t i = 0) const { return trees[i]; }

			void remove_tree(size_t i) { trees.erase(trees.begin() + i); }

			void register_handler(const ConstSoftReference<Collider>& collider, CollisionController::OverlapHandler handler, const SoftReference<CollisionController>& controller)
			{
				overlap_handlers[collider].insert(std::make_unique<OverlapHandlerRef>(controller, handler));
			}
			
			void register_handler(const ConstSoftReference<Collider>& collider, CollisionController::OverlapConstHandler handler, const ConstSoftReference<CollisionController>& controller)
			{
				overlap_handlers[collider].insert(std::make_unique<OverlapConstHandlerRef>(controller, handler));
			}
			
			void register_handler(const ConstSoftReference<Collider>& collider, CollisionController::CollisionHandler handler, const SoftReference<CollisionController>& controller)
			{
				collision_handlers[collider].insert(std::make_unique<CollisionHandlerRef>(controller, handler));
			}
			
			void register_handler(const ConstSoftReference<Collider>& collider, CollisionController::CollisionConstHandler handler, const ConstSoftReference<CollisionController>& controller)
			{
				collision_handlers[collider].insert(std::make_unique<CollisionConstHandlerRef>(controller, handler));
			}
			
			void register_handler(const ConstSoftReference<Collider>& collider, CollisionController::ContactHandler handler, const SoftReference<CollisionController>& controller)
			{
				contact_handlers[collider].insert(std::make_unique<ContactHandlerRef>(controller, handler));
			}

			void register_handler(const ConstSoftReference<Collider>& collider, CollisionController::ContactConstHandler handler, const ConstSoftReference<CollisionController>& controller)
			{
				contact_handlers[collider].insert(std::make_unique<ContactConstHandlerRef>(controller, handler));
			}

			template<std::derived_from<CollisionController> Controller>
			void register_handler(const ConstSoftReference<Collider>& collider, void(Controller::* handler)(const OverlapEventData&), const SoftReference<Controller>& controller)
			{
				register_handler(collider, static_cast<CollisionController::OverlapHandler>(handler), controller);
			}

			template<std::derived_from<CollisionController> Controller>
			void register_handler(const ConstSoftReference<Collider>& collider, void(Controller::* handler)(const OverlapEventData&) const, const ConstSoftReference<Controller>& controller)
			{
				register_handler(collider, static_cast<CollisionController::OverlapConstHandler>(handler), controller);
			}

			template<std::derived_from<CollisionController> Controller>
			void register_handler(const ConstSoftReference<Collider>& collider, void(Controller::* handler)(const CollisionEventData&), const SoftReference<Controller>& controller)
			{
				register_handler(collider, static_cast<CollisionController::CollisionHandler>(handler), controller);
			}

			template<std::derived_from<CollisionController> Controller>
			void register_handler(const ConstSoftReference<Collider>& collider, void(Controller::* handler)(const CollisionEventData&) const, const ConstSoftReference<Controller>& controller)
			{
				register_handler(collider, static_cast<CollisionController::CollisionConstHandler>(handler), controller);
			}

			template<std::derived_from<CollisionController> Controller>
			void register_handler(const ConstSoftReference<Collider>& collider, void(Controller::* handler)(const ContactEventData&), const SoftReference<Controller>& controller)
			{
				register_handler(collider, static_cast<CollisionController::ContactHandler>(handler), controller);
			}

			template<std::derived_from<CollisionController> Controller>
			void register_handler(const ConstSoftReference<Collider>& collider, void(Controller::* handler)(const ContactEventData&) const, const ConstSoftReference<Controller>& controller)
			{
				register_handler(collider, static_cast<CollisionController::ContactConstHandler>(handler), controller);
			}

			void unregister_handler(const ConstSoftReference<Collider>& collider, CollisionController::OverlapHandler handler, const SoftReference<CollisionController>& controller)
			{
				auto it = overlap_handlers.find(collider);
				if (it != overlap_handlers.end())
				{
					auto inner_it = it->second.find(std::make_unique<OverlapHandlerRef>(controller, handler));
					if (inner_it != it->second.end())
					{
						it->second.erase(inner_it);
						if (it->second.empty())
							overlap_handlers.erase(it);
					}
				}
			}

			void unregister_handler(const ConstSoftReference<Collider>& collider, CollisionController::OverlapConstHandler handler, const ConstSoftReference<CollisionController>& controller)
			{
				auto it = overlap_handlers.find(collider);
				if (it != overlap_handlers.end())
				{
					auto inner_it = it->second.find(std::make_unique<OverlapConstHandlerRef>(controller, handler));
					if (inner_it != it->second.end())
					{
						it->second.erase(inner_it);
						if (it->second.empty())
							overlap_handlers.erase(it);
					}
				}
			}

			void unregister_handler(const ConstSoftReference<Collider>& collider, CollisionController::CollisionHandler handler, const SoftReference<CollisionController>& controller)
			{
				auto it = collision_handlers.find(collider);
				if (it != collision_handlers.end())
				{
					auto inner_it = it->second.find(std::make_unique<CollisionHandlerRef>(controller, handler));
					if (inner_it != it->second.end())
					{
						it->second.erase(inner_it);
						if (it->second.empty())
							collision_handlers.erase(it);
					}
				}
			}

			void unregister_handler(const ConstSoftReference<Collider>& collider, CollisionController::CollisionConstHandler handler, const ConstSoftReference<CollisionController>& controller)
			{
				auto it = collision_handlers.find(collider);
				if (it != collision_handlers.end())
				{
					auto inner_it = it->second.find(std::make_unique<CollisionConstHandlerRef>(controller, handler));
					if (inner_it != it->second.end())
					{
						it->second.erase(inner_it);
						if (it->second.empty())
							collision_handlers.erase(it);
					}
				}
			}

			void unregister_handler(const ConstSoftReference<Collider>& collider, CollisionController::ContactHandler handler, const SoftReference<CollisionController>& controller)
			{
				auto it = contact_handlers.find(collider);
				if (it != contact_handlers.end())
				{
					auto inner_it = it->second.find(std::make_unique<ContactHandlerRef>(controller, handler));
					if (inner_it != it->second.end())
					{
						it->second.erase(inner_it);
						if (it->second.empty())
							contact_handlers.erase(it);
					}
				}
			}

			void unregister_handler(const ConstSoftReference<Collider>& collider, CollisionController::ContactConstHandler handler, const ConstSoftReference<CollisionController>& controller)
			{
				auto it = contact_handlers.find(collider);
				if (it != contact_handlers.end())
				{
					auto inner_it = it->second.find(std::make_unique<ContactConstHandlerRef>(controller, handler));
					if (inner_it != it->second.end())
					{
						it->second.erase(inner_it);
						if (it->second.empty())
							contact_handlers.erase(it);
					}
				}
			}

			template<std::derived_from<CollisionController> Controller>
			void unregister_handler(const ConstSoftReference<Collider>& collider, void(Controller::* handler)(const OverlapEventData&), const SoftReference<Controller>& controller)
			{
				unregister_handler(collider, static_cast<CollisionController::OverlapHandler>(handler), controller);
			}

			template<std::derived_from<CollisionController> Controller>
			void unregister_handler(const ConstSoftReference<Collider>& collider, void(Controller::* handler)(const OverlapEventData&) const, const ConstSoftReference<Controller>& controller)
			{
				unregister_handler(collider, static_cast<CollisionController::OverlapConstHandler>(handler), controller);
			}

			template<std::derived_from<CollisionController> Controller>
			void unregister_handler(const ConstSoftReference<Collider>& collider, void(Controller::* handler)(const CollisionEventData&), const SoftReference<Controller>& controller)
			{
				unregister_handler(collider, static_cast<CollisionController::CollisionHandler>(handler), controller);
			}

			template<std::derived_from<CollisionController> Controller>
			void unregister_handler(const ConstSoftReference<Collider>& collider, void(Controller::* handler)(const CollisionEventData&) const, const ConstSoftReference<Controller>& controller)
			{
				unregister_handler(collider, static_cast<CollisionController::CollisionConstHandler>(handler), controller);
			}

			template<std::derived_from<CollisionController> Controller>
			void unregister_handler(const ConstSoftReference<Collider>& collider, void(Controller::* handler)(const ContactEventData&), const SoftReference<Controller>& controller)
			{
				unregister_handler(collider, static_cast<CollisionController::ContactHandler>(handler), controller);
			}

			template<std::derived_from<CollisionController> Controller>
			void unregister_handler(const ConstSoftReference<Collider>& collider, void(Controller::* handler)(const ContactEventData&) const, const ConstSoftReference<Controller>& controller)
			{
				unregister_handler(collider, static_cast<CollisionController::ContactConstHandler>(handler), controller);
			}

			void unregister_overlap_handlers(const ConstSoftReference<Collider>& collider) { overlap_handlers.erase(collider); }
			void unregister_collision_handlers(const ConstSoftReference<Collider>& collider) { collision_handlers.erase(collider); }
			void unregister_contact_handlers(const ConstSoftReference<Collider>& collider) { contact_handlers.erase(collider); }
			void unregister_handlers(const ConstSoftReference<Collider>& collider) { overlap_handlers.erase(collider); collision_handlers.erase(collider); contact_handlers.erase(collider); }

			// call poll() after all collision objects have moved, but before handling events
			void poll() const;
			void clean();

			void emit(const Collider& from);
			void emit(const Collider& from, CollisionController::OverlapHandler only_handler, const SoftReference<CollisionController>& only_controller) const;
			void emit(const Collider& from, CollisionController::OverlapConstHandler only_handler, const ConstSoftReference<CollisionController>& only_controller) const;
			void emit(const Collider& from, CollisionController::CollisionHandler only_handler, const SoftReference<CollisionController>& only_controller) const;
			void emit(const Collider& from, CollisionController::CollisionConstHandler only_handler, const ConstSoftReference<CollisionController>& only_controller) const;
			void emit(const Collider& from, CollisionController::ContactHandler only_handler, const SoftReference<CollisionController>& only_controller) const;
			void emit(const Collider& from, CollisionController::ContactConstHandler only_handler, const ConstSoftReference<CollisionController>& only_controller) const;

			template<std::derived_from<CollisionController> Controller>
			void emit(const Collider& from, void(Controller::* only_handler)(const OverlapEventData&), const SoftReference<Controller>& only_controller) const
			{
				emit(from, static_cast<CollisionController::OverlapHandler>(only_handler), only_controller);
			}

			template<std::derived_from<CollisionController> Controller>
			void emit(const Collider& from, void(Controller::* only_handler)(const OverlapEventData&) const, const ConstSoftReference<Controller>& only_controller) const
			{
				emit(from, static_cast<CollisionController::OverlapConstHandler>(only_handler), only_controller);
			}

			template<std::derived_from<CollisionController> Controller>
			void emit(const Collider& from, void(Controller::* only_handler)(const CollisionEventData&), const SoftReference<Controller>& only_controller) const
			{
				emit(from, static_cast<CollisionController::CollisionHandler>(only_handler), only_controller);
			}

			template<std::derived_from<CollisionController> Controller>
			void emit(const Collider& from, void(Controller::* only_handler)(const CollisionEventData&) const, const ConstSoftReference<Controller>& only_controller) const
			{
				emit(from, static_cast<CollisionController::CollisionConstHandler>(only_handler), only_controller);
			}

			template<std::derived_from<CollisionController> Controller>
			void emit(const Collider& from, void(Controller::* only_handler)(const ContactEventData&), const SoftReference<Controller>& only_controller) const
			{
				emit(from, static_cast<CollisionController::ContactHandler>(only_handler), only_controller);
			}

			template<std::derived_from<CollisionController> Controller>
			void emit(const Collider& from, void(Controller::* only_handler)(const ContactEventData&) const, const ConstSoftReference<Controller>& only_controller) const
			{
				emit(from, static_cast<CollisionController::ContactConstHandler>(only_handler), only_controller);
			}
		};
	}
}
