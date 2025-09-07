#include "ColliderDispatchHandle.h"

#include "core/context/Collision.h"
#include "physics/collision/scene/dispatch/CollisionDispatcher.h"
#include "physics/collision/scene/colliders/Collider.h"

namespace oly::col2d::internal
{
	ColliderDispatchHandle::ColliderDispatchHandle(const Collider& collider)
		: collider(collider)
	{
	}

	ColliderDispatchHandle::ColliderDispatchHandle(const Collider& collider, const ColliderDispatchHandle& other)
		: collider(collider)
	{
		copy_handlers(other);
	}

	ColliderDispatchHandle::ColliderDispatchHandle(const Collider& collider, ColliderDispatchHandle&& other) noexcept
		: collider(collider)
	{
		move_handlers(std::move(other));
	}

	ColliderDispatchHandle::~ColliderDispatchHandle()
	{
		remove_handlers();
	}

	ColliderDispatchHandle& ColliderDispatchHandle::operator=(const ColliderDispatchHandle& other)
	{
		if (this != &other)
		{
			remove_handlers();
			copy_handlers(other);
		}
		return *this;
	}

	ColliderDispatchHandle& ColliderDispatchHandle::operator=(ColliderDispatchHandle&& other) noexcept
	{
		if (this != &other)
		{
			remove_handlers();
			move_handlers(std::move(other));
		}
		return *this;
	}

	template<typename Map, typename LUT>
	static void copy_dispatch_handle(const Collider& collider, const Collider& other_collider, Map& handler_map, LUT& controller_lut)
	{
		auto it = handler_map.find(&other_collider);
		if (it != handler_map.end())
		{
			auto& copy_set = handler_map[&collider];
			for (const auto& handler : it->second)
			{
				copy_set.insert(handler->clone());
				auto& lut_set = controller_lut.find(handler->controller)->second;
				lut_set.insert(std::make_pair(&collider, handler->clone()));
			}
		}
	}

	void ColliderDispatchHandle::copy_handlers(const ColliderDispatchHandle& other)
	{
		auto& dispatcher = context::collision_dispatcher();
		copy_dispatch_handle(collider, other.collider, dispatcher.overlap_handler_map, dispatcher.overlap_controller_lut);
		copy_dispatch_handle(collider, other.collider, dispatcher.collision_handler_map, dispatcher.collision_controller_lut);
		copy_dispatch_handle(collider, other.collider, dispatcher.contact_handler_map, dispatcher.contact_controller_lut);
		dispatcher.phase_tracker.copy_all(other.collider, collider);
		dispatcher.collision_cache.copy_all(other.collider, collider);
	}

	template<typename Map, typename LUT>
	static void move_dispatch_handle(const Collider& collider, const Collider& other_collider, Map& handler_map, LUT& controller_lut)
	{
		auto it = handler_map.find(&other_collider);
		if (it != handler_map.end())
		{
			for (auto& handler : it->second)
			{
				auto& lut_set = controller_lut.find(handler->controller)->second;
				lut_set.erase(std::make_pair(&other_collider, handler->clone()));
				lut_set.insert(std::make_pair(&collider, handler->clone()));
			}
			handler_map[&collider] = std::move(it->second);
			handler_map.erase(it);
		}
	}

	void ColliderDispatchHandle::move_handlers(ColliderDispatchHandle&& other)
	{
		auto& dispatcher = context::collision_dispatcher();
		move_dispatch_handle(collider, other.collider, dispatcher.overlap_handler_map, dispatcher.overlap_controller_lut);
		move_dispatch_handle(collider, other.collider, dispatcher.collision_handler_map, dispatcher.collision_controller_lut);
		move_dispatch_handle(collider, other.collider, dispatcher.contact_handler_map, dispatcher.contact_controller_lut);
		dispatcher.phase_tracker.replace_all(collider, other.collider);
		dispatcher.collision_cache.replace_all(collider, other.collider);
	}

	template<typename Map, typename LUT>
	static void remove_dispatch_handle(const Collider& collider, Map& handler_map, LUT& controller_lut)
	{
		auto it = handler_map.find(&collider);
		if (it != handler_map.end())
		{
			for (const auto& handler : it->second)
			{
				auto& lut_set = controller_lut.find(handler->controller)->second;
				lut_set.erase(std::make_pair(&collider, handler->clone()));
			}
			handler_map.erase(it);
		}
	}

	void ColliderDispatchHandle::remove_handlers()
	{
		auto& dispatcher = context::collision_dispatcher();
		remove_dispatch_handle(collider, dispatcher.overlap_handler_map, dispatcher.overlap_controller_lut);
		remove_dispatch_handle(collider, dispatcher.collision_handler_map, dispatcher.collision_controller_lut);
		remove_dispatch_handle(collider, dispatcher.contact_handler_map, dispatcher.contact_controller_lut);
		dispatcher.phase_tracker.erase_all(collider);
		dispatcher.collision_cache.erase_all(collider);
	}
}

