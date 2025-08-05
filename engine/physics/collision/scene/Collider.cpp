#include "Collider.h"

#include "core/context/Collision.h"
#include "physics/dynamics/bodies/RigidBody.h"

namespace oly::col2d
{
	namespace internal
	{
		TreeHandleMap::TreeHandleMap(Collider& collider, const TreeHandleMap& other)
			: collider(collider), handles(other.handles)
		{
			for (auto& [tree, node] : handles)
				if (node)
					node->set_colliders().insert(&collider);
		}

		TreeHandleMap::TreeHandleMap(Collider& collider, TreeHandleMap&& other) noexcept
			: collider(collider), handles(std::move(other.handles))
		{
			for (auto& [tree, node] : handles)
				if (node)
					node->set_colliders().replace(&other.collider, &collider);
		}

		TreeHandleMap::~TreeHandleMap()
		{
			clear();
		}

		TreeHandleMap& TreeHandleMap::operator=(const TreeHandleMap& other)
		{
			if (this != &other)
			{
				for (auto it = handles.begin(); it != handles.end(); )
				{
					if (other.handles.count(it->key))
						++it;
					else
						it = handles.erase(it);
				}

				for (auto it = other.handles.begin(); it != other.handles.end(); ++it)
				{
					if (!handles.count(it->key))
					{
						if (it->value)
							it->value->set_colliders().insert(&collider);
						handles.insert(it->key, it->value);
					}
				}
			}
			return *this;
		}

		TreeHandleMap& TreeHandleMap::operator=(TreeHandleMap&& other) noexcept
		{
			if (this != &other)
			{
				clear();
				handles = std::move(other.handles);
				for (auto& [tree, node] : handles)
					if (node)
						node->set_colliders().replace(&other.collider, &collider);
			}
			return *this;
		}

		void TreeHandleMap::flush() const
		{
			for (auto& [tree, node] : handles)
			{
				if (node)
					node->update(collider, node);
				else
				{
					tree->root->set_colliders().insert(&collider);
					node = tree->root.get();
				}
			}
		}

		void TreeHandleMap::attach(const CollisionTree& tree)
		{
			if (!handles.count(&tree))
			{
				tree.root->set_colliders().insert(&collider);
				handles.insert(&tree, tree.root.get());
			}
		}

		void TreeHandleMap::attach(size_t context_tree_index)
		{
			attach(context::collision_dispatcher().get_tree(context_tree_index));
		}

		void TreeHandleMap::detach(const CollisionTree& tree)
		{
			auto it = handles.find(&tree);
			if (it != handles.end())
			{
				if (it->value)
					it->value->set_colliders().erase(&collider);
				handles.erase(it);
			}
		}

		void TreeHandleMap::detach(size_t context_tree_index)
		{
			detach(context::collision_dispatcher().get_tree(context_tree_index));
		}

		bool TreeHandleMap::is_attached(size_t context_tree_index) const
		{
			return is_attached(context::collision_dispatcher().get_tree(context_tree_index));
		}

		void TreeHandleMap::clear()
		{
			for (auto& [tree, node] : handles)
				if (node)
					node->set_colliders().erase(&collider);
			handles.clear();
		}

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
		}
	}

	Collider::Collider(const Collider& other)
		: obj(other.obj), handles(*this, other.handles), dirty(other.dirty), dispatch_handle(*this, other.dispatch_handle), quad_wrap(other.quad_wrap)
	{
	}

	Collider::Collider(Collider&& other) noexcept
		: obj(std::move(other.obj)), handles(*this, std::move(other.handles)), dirty(other.dirty), dispatch_handle(*this, std::move(other.dispatch_handle)), quad_wrap(other.quad_wrap)
	{
	}

	Collider& Collider::operator=(const Collider& other)
	{
		if (this != &other)
		{
			obj = other.obj;
			dirty = other.dirty;
			dispatch_handle = other.dispatch_handle;
			quad_wrap = other.quad_wrap;
			handles = other.handles;
		}
		return *this;
	}

	Collider& Collider::operator=(Collider&& other) noexcept
	{
		if (this != &other)
		{
			obj = std::move(other.obj);
			dirty = other.dirty;
			dispatch_handle = std::move(other.dispatch_handle);
			quad_wrap = other.quad_wrap;
			handles = std::move(other.handles);
		}
		return *this;
	}

	void Collider::flush() const
	{
		if (!is_dirty())
			return;

		dirty = false;
		internal::lut_flush(obj);
		handles.flush();
	}
}
