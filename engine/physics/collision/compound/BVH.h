#pragma once

#include "physics/collision/compound/Compound.h"
#include "core/math/Solvers.h"
#include "core/base/Assert.h"

namespace oly::col2d
{
	namespace internal
	{
		template<typename Shape>
		struct Wrap
		{
			Shape operator()(const Primitive* primitives, size_t count) const { static_assert(false, "oly::col2d::internal::wrap not defined for the provided Shape"); }
		};

		template<>
		struct Wrap<AABB>
		{
			AABB operator()(const Primitive* primitives, size_t count) const;
		};

		template<>
		struct Wrap<OBB>
		{
			OBB operator()(const Primitive* primitives, size_t count) const;
		};

		// TODO other shapes
	}

	template<typename Shape>
	class BVH
	{
		struct Node
		{
			std::optional<Shape> shape;
			size_t start_index;
			std::unique_ptr<Node> left, right;

			Node(const Primitive* primitives, size_t start_index, size_t count)
				: start_index(start_index)
			{
				OLY_ASSERT(count > 0);
				if (count > 1)
				{
					shape.emplace(internal::Wrap<Shape>{}(primitives + start_index, count));
					left = std::make_unique<Node>(primitives, start_index, (count + 1) / 2);
					right = std::make_unique<Node>(primitives, start_index + (count + 1) / 2, count / 2);
				}
			}

			bool is_leaf() const { return !shape.has_value(); }
		};

		mutable std::unique_ptr<Node> _root;
		std::vector<Primitive> primitives;
		mutable bool dirty = true;

	public:
		const std::vector<Primitive>& get_primitives() const { return primitives; }
		std::vector<Primitive>& set_primitives() { dirty = true; return primitives; }

	private:
		const Node& root() const
		{
			if (dirty)
			{
				dirty = false;
				// TODO later, sort primitives by heuristic, such as left-to-right
				_root = std::make_unique<Node>(primitives.data(), 0, primitives.size());
			}
			return *_root;
		}

	public:
		OverlapResult point_hits(glm::vec2 test) const { return point_hits(root(), primitives, test); }
		OverlapResult ray_hits(const Ray& ray) const { return ray_hits(root(), primitives, ray); }
		RaycastResult raycast(const Ray& ray) const { return raycast(root(), primitives, ray); }
		
		OverlapResult overlaps(const Primitive& c) const { return overlaps(root(), primitives, c); }
		OverlapResult overlaps(const Compound& c) const { return overlaps(root(), primitives, c); }
		OverlapResult overlaps(const TCompound& c) const { return overlaps(root(), primitives, c); }
		template<typename S>
		OverlapResult overlaps(const BVH<S>& bvh) const { return overlaps(root(), primitives, bvh.root(), bvh.primitives.data()); }
		
		CollisionResult collides(const Primitive& c) const { return collides(root(), primitives, c); }
		CollisionResult collides(const Compound& c) const { return collides(root(), primitives, c); }
		CollisionResult collides(const TCompound& c) const { return collides(root(), primitives, c); }
		template<typename S>
		CollisionResult collides(const BVH<S>& bvh) const { return collides(root(), primitives, bvh.root()); }
		
		ContactResult contacts(const Primitive& c) const { return contacts(root(), primitives, c); }
		ContactResult contacts(const Compound& c) const { return contacts(root(), primitives, c); }
		ContactResult contacts(const TCompound& c) const { return contacts(root(), primitives, c); }
		template<typename S>
		ContactResult contacts(const BVH<S>& bvh) const { return contacts(root(), primitives, bvh.root()); }

	private:
		static OverlapResult point_hits(const Node& node, const Primitive* primitives, glm::vec2 test)
		{
			if (node.is_leaf())
				return col2d::point_hits(primitives[node.start_index], test);
			else if (!col2d::point_hits(node.shape.value(), test))
				return false;
			else
				return point_hits(*node.left, test) || point_hits(*node.right, test);
		}

		static OverlapResult ray_hits(const Node& node, const Primitive* primitives, const Ray& ray)
		{
			if (node.is_leaf())
				return col2d::ray_hits(primitives[node.start_index], ray);
			else if (!col2d::ray_hits(node.shape.value(), ray))
				return false;
			else
				return ray_hits(*node.left, ray) || ray_hits(*node.right, ray);
		}

		static RaycastResult raycast(const Node& node, const Primitive* primitives, const Ray& ray)
		{
			if (node.is_leaf())
				return col2d::raycast(primitives[node.start_index], ray);
			else if (!col2d::ray_hits(node.shape.value(), ray))
				return { .hit = RaycastResult::Hit::NO_HIT };
			else
			{
				RaycastResult left_result = raycast(*node.left, ray);
				if (left_result.hit == RaycastResult::Hit::EMBEDDED_ORIGIN)
					return left_result;
				RaycastResult right_result = raycast(*node.right, ray);
				if (right_result.hit == RaycastResult::Hit::EMBEDDED_ORIGIN)
					return right_result;
				
				if (left_result.hit == RaycastResult::Hit::NO_HIT)
				{
					if (right_result.hit == RaycastResult::Hit::NO_HIT)
						return { .hit = RaycastResult::Hit::NO_HIT };
					else
						return right_result;
				}
				else
				{
					if (right_result.hit == RaycastResult::Hit::NO_HIT)
						return left_result;
					else
						return math::mag_sqrd(left_result.contact - ray.origin) < math::mag_sqrd(right_result.contact - ray.origin) ? left_result : right_result;
				}
			}
		}

		template<typename OtherShape>
		static OverlapResult overlaps(const Node& my_node, const Primitive* my_primitives, const BVH<OtherShape>::Node& other_node, const Primitive* other_primitives)
		{
			if (my_node.is_leaf())
			{
				if (other_node.is_leaf())
					return col2d::overlaps(my_primitives[my_node.start_index], other_primitives[other_node.start_index]);
				else
					return BVH<OtherShape>::overlaps(other_node, other_primitives, my_primitives[my_node.start_index]);
			}
			else
			{
				if (other_node.is_leaf())
					return overlaps(my_node, my_primitives, other_primitives[other_node.start_index]);
				else
				{
					if (!col2d::overlaps(my_node.shape.value(), other_node.shape.value()))
						return false;
					else
						return overlaps(*my_node.left, my_primitives, *other_node.left, other_primitives)
							|| overlaps(*my_node.left, my_primitives, *other_node.right, other_primitives)
							|| overlaps(*my_node.right, my_primitives, *other_node.left, other_primitives)
							|| overlaps(*my_node.right, my_primitives, *other_node.right, other_primitives);
				}
			}
		}

		template<typename Other>
		static OverlapResult overlaps(const Node& node, const Primitive* primitives, const Other& c)
		{
			if (node.is_leaf())
				return overlaps(primitives[node.start_index], c);
			else if (!col2d::overlaps(node.shape.value(), c))
				return false;
			else
				return overlaps(*node.left, c) || overlaps(*node.right, c);
		}

		template<typename OtherShape>
		static CollisionResult collides(const Node& my_node, const Primitive* my_primitives, const BVH<OtherShape>::Node& other_node, const Primitive* other_primitives)
		{
			if (my_node.is_leaf())
			{
				if (other_node.is_leaf())
					return collides(my_primitives[my_node.start_index], other_primitives[other_node.start_index]);
				else
					return BVH<OtherShape>::collides(other_node, other_primitives, my_primitives[my_node.start_index]).invert();
			}
			else
			{
				if (other_node.is_leaf())
					return collides(my_node, my_primitives, other_primitives[other_node.start_index]);
				else
				{
					if (!col2d::overlaps(my_node.shape.value(), other_node.shape.value()))
						return false;
					else
						return greedy_collision({ collides(*my_node.left,  my_primitives, *other_node.left, other_primitives), collides(*my_node.left,  my_primitives, *other_node.right, other_primitives),
												  collides(*my_node.right, my_primitives, *other_node.left, other_primitives), collides(*my_node.right, my_primitives, *other_node.right, other_primitives) });
				}
			}
		}

		template<typename Other>
		static CollisionResult collides(const Node& node, const Other& c)
		{
			if (node.is_leaf())
				return collides(primitives[node.start_index], c);
			else if (!col2d::overlaps(node.shape.value(), c))
				return { .overlap = false };
			else
				return greedy_collision({ collides(*node.left, c), collides(*node.right, c) });
		}

		template<typename OtherShape>
		static ContactResult contacts(const Node& my_node, const Primitive* my_primitives, const BVH<OtherShape>::Node& other_node, const Primitive* other_primitives)
		{
			if (my_node.is_leaf())
			{
				if (other_node.is_leaf())
					return contacts(my_primitives[my_node.start_index], other_primitives[other_node.start_index]);
				else
					return BVH<OtherShape>::contacts(other_node, other_primitives, my_primitives[my_node.start_index]).invert();
			}
			else
			{
				if (other_node.is_leaf())
					return contacts(my_node, my_primitives, other_primitives[other_node.start_index]);
				else
					return greedy_contact({ contacts(*my_node.left,  my_primitives, *other_node.left, other_primitives), contacts(*my_node.left,  my_primitives, *other_node.right, other_primitives),
											contacts(*my_node.right, my_primitives, *other_node.left, other_primitives), contacts(*my_node.right, my_primitives, *other_node.right, other_primitives) });
			}
		}

		template<typename Other>
		static ContactResult contacts(const Node& node, const Primitive* primitives, const Other& c)
		{
			if (node.is_leaf())
				return contacts(primitives[node.start_index], c);
			else if (!col2d::overlaps(node.shape.value(), c))
				return { .overlap = false };
			else
				return greedy_contact({ contacts(*node.left, c), contacts(*node.right, c) });
		}
	};

	template<typename Shape>
	class TBVH
	{
		mutable BVH<Shape> _bvh;
		mutable bool local_dirty = true;
		std::vector<Primitive> local_primitives;

		const BVH<Shape> bvh() const
		{
			if (local_dirty)
			{
				local_dirty = false;
				const glm::mat3 m = transformer.global();
				std::vector<Primitive>& global_primitives = _bvh.set_primitives();
				global_primitives.resize(local_primitives.size());
				for (size_t i = 0; i < local_primitives.size(); ++i)
					global_primitives[i] = transform_primitive(local_primitives[i], m);
			}
			return _bvh;
		}

	public:
		Transformer2D transformer;

		const Transform2D& get_local() const { return transformer.get_local(); }
		Transform2D& set_local() { local_dirty = true; return transformer.get_local(); }

		const std::vector<Primitive>& get_primitives() const { return local_primitives; }
		std::vector<Primitive>& set_primitives() { local_dirty = true; return local_primitives; }

		OverlapResult point_hits(glm::vec2 test) const { return bvh().point_hits(test); }
		OverlapResult ray_hits(const Ray& ray) const { return bvh().ray_hits(ray); }
		RaycastResult raycast(const Ray& ray) const { return bvh().raycast(ray); }

		OverlapResult overlaps(const Primitive& c) const { return bvh().overlaps(c); }
		OverlapResult overlaps(const Compound& c) const { return bvh().overlaps(c); }
		OverlapResult overlaps(const TCompound& c) const { return bvh().overlaps(c); }
		template<typename S>
		OverlapResult overlaps(const BVH<S>& c) const { return bvh().overlaps(c); }
		template<typename S>
		OverlapResult overlaps(const TBVH<S>& c) const { return bvh().overlaps(c.bvh()); }

		CollisionResult collides(const Primitive& c) const { return bvh().collides(c); }
		CollisionResult collides(const Compound& c) const { return bvh().collides(c); }
		CollisionResult collides(const TCompound& c) const { return bvh().collides(c); }
		template<typename S>
		CollisionResult collides(const BVH<S>& c) const { return bvh().collides(c); }
		template<typename S>
		CollisionResult collides(const TBVH<S>& c) const { return bvh().collides(c.bvh()); }

		ContactResult contacts(const Primitive& c) const { return bvh().contacts(c); }
		ContactResult contacts(const Compound& c) const { return bvh().contacts(c); }
		ContactResult contacts(const TCompound& c) const { return bvh().contacts(c); }
		template<typename S>
		ContactResult contacts(const BVH<S>& c) const { return bvh().contacts(c); }
		template<typename S>
		ContactResult contacts(const TBVH<S>& c) const { return bvh().contacts(c.bvh()); }
	};
}
