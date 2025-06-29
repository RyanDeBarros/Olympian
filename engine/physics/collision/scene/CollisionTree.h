#pragma once

#include "core/containers/FixedVector.h"
#include "core/math/Shapes.h"

#include <memory>

namespace oly::col2d
{
	struct ICollider
	{
		virtual ~ICollider() = default;
		virtual math::Rect2D quad_wrap() const = 0;
	};

	class CollisionNode;
	class CollisionTree;

	// It is important that the handle cannot outlive the collider it references, or else it will be dangling.
	class CollisionHandle
	{
		friend class CollisionNode;

		const ICollider* collider = nullptr;
		CollisionNode* node = nullptr;

		CollisionHandle(const ICollider* collider = nullptr, CollisionNode* node = nullptr);

	public:
		CollisionHandle(const CollisionHandle&) = delete;
		CollisionHandle(CollisionHandle&& other) noexcept;
		~CollisionHandle();
		CollisionHandle& operator=(const CollisionHandle&) = delete;
		CollisionHandle& operator=(CollisionHandle&& other) noexcept;
	};

	class CollisionNode
	{
		friend class CollisionTree;
		friend class CollisionHandle;

		const CollisionTree* tree = nullptr;
		math::Rect2D bounds;
		FixedVector<std::unique_ptr<CollisionNode>> subnodes;
		std::vector<const ICollider*> colliders;

		CollisionNode(const CollisionTree* tree);
		static std::unique_ptr<CollisionNode> instantiate(const CollisionTree* tree);

	public:
		CollisionHandle insert(const ICollider* collider);

	private:
		CollisionHandle insert(const ICollider& collider, const math::Rect2D& b);

		bool subnode_coordinates(const math::Rect2D& b, unsigned int& x, unsigned int& y) const;
		const std::unique_ptr<CollisionNode>& subnode(unsigned int x, unsigned int y) const;
		std::unique_ptr<CollisionNode>& subnode(unsigned int x, unsigned int y);

		void remove_direct(const ICollider* collider);

	public:
		bool remove(const ICollider* collider);

	private:
		bool remove(const ICollider& collider, const math::Rect2D& b);

		math::Rect2D subdivision(int x, int y) const;
	};

	class CollisionTree
	{
		friend class CollisionNode;

		const size_t cell_capacity = 4;
		const glm::uvec2 degree;
		const glm::vec2 inv_degree;

		std::unique_ptr<CollisionNode> root;

	public:
		CollisionTree(math::Rect2D bounds, glm::uvec2 degree = { 2, 2 }, size_t cell_capacity = 4);

		CollisionHandle insert(const ICollider* collider);
	};
}
