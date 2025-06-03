#pragma once

#include "physics/collision/compound/Compound.h"
#include "core/math/Solvers.h"

namespace oly::col2d
{
	namespace internal
	{
		// TODO Primitive for Element. BVH-Compound and TBVH-TCompound conversion.

		template<typename Shape, typename Element>
		struct Wrap
		{
			Shape operator()(const Element* elements, size_t count)
					{ static_assert(false, "oly::col2d::internal::wrap not defined for the provided Shape-Element configuration."); }
		};

		template<>
		struct Wrap<AABB, glm::vec2>
		{
			AABB operator()(const glm::vec2* elements, size_t count)
			{
				return AABB::wrap(elements, count);
			}
		};

		template<size_t N>
		struct Wrap<AABB, std::array<glm::vec2, N>>
		{
			AABB operator()(const std::array<glm::vec2, N>* elements, size_t count)
			{
				AABB c = AABB::DEFAULT;
				for (size_t i = 0; i < count; ++i)
				{
					for (size_t j = 0; j < N; ++j)
					{
						c.x1 = std::min(c.x1, elements[i][j].x);
						c.x2 = std::max(c.x2, elements[i][j].x);
						c.y1 = std::min(c.y1, elements[i][j].y);
						c.y2 = std::max(c.y2, elements[i][j].y);
					}
				}
				return c;
			}
		};

		template<>
		struct Wrap<OBB, glm::vec2>
		{
			OBB operator()(const glm::vec2* elements, size_t count)
			{
				return OBB::fast_wrap(elements, count);
			}
		};

		template<size_t N>
		struct Wrap<OBB, std::array<glm::vec2, N>>
		{
			OBB operator()(const std::array<glm::vec2, N>* elements, size_t count)
			{
				OBB obb{};

				glm::vec2 centroid = {};
				for (size_t i = 0; i < count; ++i)
					for (size_t j = 0; j < N; ++j)
						centroid += elements[i][j];
				centroid /= (float)count;

				math::solver::Eigen2x2 covariance{ .M = 0.0f };

				for (size_t i = 0; i < count; ++i)
				{
					for (size_t j = 0; j < N; ++j)
					{
						glm::vec2 p = elements[i][j] - centroid;
						covariance.M[0][0] += p.x * p.x;
						covariance.M[0][1] += p.x * p.y;
						covariance.M[1][0] += p.y * p.x;
						covariance.M[1][1] += p.y * p.y;
					}
				}
				covariance.M /= (float)count;

				glm::vec2 eigenvectors[2];
				covariance.solve(nullptr, eigenvectors);
				glm::vec2 major_axis = eigenvectors[1];
				glm::vec2 minor_axis = eigenvectors[0];

				obb.rotation = glm::atan(major_axis.y, major_axis.x);

				AABB bounds = AABB::DEFAULT;
				for (size_t i = 0; i < count; ++i)
				{
					for (size_t j = 0; j < N; ++j)
					{
						glm::vec2 p = elements[i][j] - centroid;
						float x = glm::dot(p, major_axis);
						float y = glm::dot(p, minor_axis);

						bounds.x1 = std::min(bounds.x1, x);
						bounds.x2 = std::max(bounds.x2, x);
						bounds.y1 = std::min(bounds.y1, y);
						bounds.y2 = std::max(bounds.y2, y);
					}
				}

				return { .center = bounds.center(), .width = bounds.x2 - bounds.x1, .height = bounds.y2 - bounds.y1 };
			}
		};
	}

	template<typename Shape, typename Element>
	class BVH
	{
		struct Node
		{
			Shape shape;
			size_t start_index;
			std::unique_ptr<Node> left, right;

			Node(const Element* elements, size_t start_index, size_t count)
				: start_index(start_index), shape(internal::Wrap<Shape, Element>{}(elements + start_index, count))
			{
				if (count > 0)
					left = std::make_unique<Node>(elements, start_index, (count + 1) / 2);
				if (count > 1)
					right = std::make_unique<Node>(elements, start_index + (count + 1) / 2, count / 2);
			}

			bool is_leaf() const { return !left && !right; }
		};

		mutable std::unique_ptr<Node> _root;
		std::vector<Element> elements;
		mutable bool dirty = true;

		template<typename S, typename E>
		friend class BVH<S, E>;

	public:
		const std::vector<Element>& get_elements() const { return elements; }
		std::vector<Element>& set_elements() { dirty = true; return elements; }

	private:
		const Node& root() const
		{
			if (dirty)
			{
				dirty = false;
				// TODO later, sort elements by heuristic, such as left-to-right
				_root = std::make_unique<Node>(elements.data(), 0, elements.size());
			}
			return *_root;
		}

	public:
		OverlapResult point_hits(glm::vec2 test) const { return point_hits(root(), test); }
		OverlapResult ray_hits(const Ray& ray) const { return ray_hits(root(), ray); }
		RaycastResult raycast(const Ray& ray) const { return raycast(root(), ray); }
		
		OverlapResult overlaps(const Primitive& c) const;
		OverlapResult overlaps(const Compound& c) const;
		OverlapResult overlaps(const TCompound& c) const;
		template<typename E, typename S>
		OverlapResult overlaps(const BVH<E, S>& bvh) const;
		
		CollisionResult collides(const Primitive& c) const;
		CollisionResult collides(const Compound& c) const;
		CollisionResult collides(const TCompound& c) const;
		template<typename E, typename S>
		CollisionResult collides(const BVH<E, S>& bvh) const;
		
		ContactResult contacts(const Primitive& c) const;
		ContactResult contacts(const Compound& c) const;
		ContactResult contacts(const TCompound& c) const;
		template<typename E, typename S>
		ContactResult contacts(const BVH<E, S>& bvh) const;

	private:
		OverlapResult point_hits(const Node& node, glm::vec2 test) const
		{
			if (!col2d::point_hits(node.shape, test))
				return false;
			else if (node.is_leaf())
				return col2d::point_hits(elements[node.start_index], test); // TODO this is redundant for ConvexHull Shape wrapping convex polygon elements
			else
				return (node.left && point_hits(*node.left, test)) || (node.right && point_hits(*node.right, test));
		}

		OverlapResult ray_hits(const Node& node, const Ray& ray) const
		{
			if (!col2d::ray_hits(node.shape, ray))
				return false;
			else if (node.is_leaf())
				return col2d::ray_hits(elements[node.start_index], ray); // TODO this is redundant for ConvexHull Shape wrapping convex polygon elements
			else
				return (node.left && ray_hits(*node.left, ray)) || (node.right && ray_hits(*node.right, ray));
		}

		RaycastResult raycast(const Node& node, const Ray& ray) const
		{
			if (!col2d::ray_hits(node.shape, ray))
				return { .hit = RaycastResult::Hit::NO_HIT };
			if (node.is_leaf())
				return col2d::raycast(elements[node.start_index], ray);
			else // node.left == true
			{
				RaycastResult left_result = raycast(*node.left, ray);
				if (left_result.hit == RaycastResult::Hit::EMBEDDED_ORIGIN)
					return left_result;
				else if (node.right)
				{
					RaycastResult right_result = raycast(*node.right, ray);
					if (right_result.hit == RaycastResult::Hit::EMBEDDED_ORIGIN)
						return right_result;
					else if (right_result.hit == RaycastResult::Hit::TRUE_HIT)
					{
						if (left_result.hit == RaycastResult::Hit::NO_HIT)
							return right_result;
						else // left_result.hit == RaycastResult::Hit::TRUE_HIT
							return math::mag_sqrd(left_result.contact - ray.origin) < math::mag_sqrd(right_result.contact - ray.origin) ? left_result : right_result;
					}
				}
				return left_result;
			}
		}
	};

	inline glm::vec2 transform_primitive(glm::vec2 c, const glm::mat3& m)
	{
		return transform_point(m, c);
	}

	template<size_t N>
	inline std::array<glm::vec2, N> transform_primitive(const std::array<glm::vec2, N>& c, const glm::mat3& m)
	{
		std::array<glm::vec2, N> global;
		for (size_t i = 0; i < N; ++i)
			global[i] = transform_point(m, c[i]);
		return global;
	}

	template<typename Shape, typename Element>
	class TBVH
	{
		mutable BVH _bvh;
		mutable bool local_dirty = true;
		std::vector<Element> local_elements;

		template<typename S, typename E>
		friend class TBVH<S, E>;

		const BVH bvh() const
		{
			if (local_dirty)
			{
				local_dirty = false;
				const glm::mat3 m = transformer.global();
				std::vector<Element>& global_elements = _bvh.set_elements();
				global_elements.resize(local_elements.size());
				for (size_t i = 0; i < local_elements.size(); ++i)
					global_elements[i] = transform_primitive(local_elements[i], m);
			}
			return _bvh;
		}

	public:
		Transformer2D transformer;

		const Transform2D& get_local() const { return transformer.get_local(); }
		Transform2D& set_local() { local_dirty = true; return transformer.get_local(); }

		const std::vector<Element>& get_elements() const { return local_elements; }
		std::vector<Element>& set_elements() { local_dirty = true; return local_elements; }

		OverlapResult point_hits(glm::vec2 test) const { return bvh().point_hits(test); }
		OverlapResult ray_hits(const Ray& ray) const { return bvh().ray_hits(ray); }
		RaycastResult raycast(const Ray& ray) const { return bvh().raycast(ray); }

		OverlapResult overlaps(const Primitive& c) const;
		OverlapResult overlaps(const Compound& c) const;
		OverlapResult overlaps(const TCompound& c) const;
		template<typename E, typename S>
		OverlapResult overlaps(const BVH<E, S>& bvh) const;
		template<typename E, typename S>
		OverlapResult overlaps(const TBVH<E, S>& bvh) const;

		CollisionResult collides(const Primitive& c) const;
		CollisionResult collides(const Compound& c) const;
		CollisionResult collides(const TCompound& c) const;
		template<typename E, typename S>
		CollisionResult collides(const BVH<E, S>& bvh) const;
		template<typename E, typename S>
		CollisionResult collides(const TBVH<E, S>& bvh) const;

		ContactResult contacts(const Primitive& c) const;
		ContactResult contacts(const Compound& c) const;
		ContactResult contacts(const TCompound& c) const;
		template<typename E, typename S>
		ContactResult contacts(const BVH<E, S>& bvh) const;
		template<typename E, typename S>
		ContactResult contacts(const TBVH<E, S>& bvh) const;
	};
}
