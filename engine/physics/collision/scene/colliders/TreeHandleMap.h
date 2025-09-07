#pragma once

#include "core/containers/ContiguousMap.h"

namespace oly::col2d
{
	class Collider;
	class CollisionTree;

	namespace internal
	{
		class CollisionNode;
	}
}

namespace oly::col2d::internal
{
	class TreeHandleMap
	{
		friend class internal::CollisionNode;
		friend class CollisionTree;
		friend class Collider;
		Collider& collider;
		mutable ContiguousMap<const CollisionTree*, internal::CollisionNode*> handles;

		TreeHandleMap(Collider& collider) : collider(collider) {}
		TreeHandleMap(const TreeHandleMap&) = delete;
		TreeHandleMap(TreeHandleMap&&) = delete;
		TreeHandleMap(Collider&, const TreeHandleMap&);
		TreeHandleMap(Collider&, TreeHandleMap&&) noexcept;
		~TreeHandleMap();

		TreeHandleMap& operator=(const TreeHandleMap&);
		TreeHandleMap& operator=(TreeHandleMap&&) noexcept;

		void flush() const;

	public:
		void attach(const CollisionTree& tree);
		void attach(size_t context_tree_index = 0);
		void detach(const CollisionTree& tree);
		void detach(size_t context_tree_index = 0);
		bool is_attached(const CollisionTree& tree) const { return handles.count(&tree); }
		bool is_attached(size_t context_tree_index = 0) const;
		void clear();
		size_t size() const { return handles.size(); }
	};
}
