#pragma once

#include "physics/collision/elements/Element.h"
#include "physics/collision/methods/Collide.h"
#include "physics/collision/methods/KDOPCollide.h"
#include "core/base/Transforms.h"
#include "core/math/Solvers.h"
#include "core/base/Assert.h"
#include "physics/collision/Tolerance.h"

#include <algorithm>

namespace oly::col2d
{
	namespace internal
	{
		template<typename Shape>
		struct Wrap
		{
			Shape operator()(const Element* elements, size_t count) const { static_assert(false, "oly::col2d::internal::wrap not defined for the provided Shape"); }
		};

		template<>
		struct Wrap<AABB>
		{
			AABB operator()(const Element* elements, size_t count) const;
		};

		template<>
		struct Wrap<OBB>
		{
			OBB operator()(const Element* elements, size_t count) const;
		};

		template<>
		struct Wrap<Circle>
		{
			Circle operator()(const Element* elements, size_t count) const;
		};

		template<>
		struct Wrap<ConvexHull>
		{
			class CirclePolygonEnclosure
			{
				size_t degree = 8;
				float overfitting_multiplier;

				void set_mult() { overfitting_multiplier = 1.0f / glm::cos(glm::pi<float>() / float(degree)); }

			public:
				CirclePolygonEnclosure(size_t degree = 8) : degree(degree) { set_mult(); }

				size_t get_degree() const { return degree; }
				void set_degree(size_t d) { degree = d; set_mult(); }
				glm::vec2 get_point(const Circle& c, size_t i) const
				{
					return c.center + c.radius * overfitting_multiplier * (glm::vec2)UnitVector2D(i * glm::two_pi<float>() / get_degree());
				}
			};

			static CirclePolygonEnclosure CIRCLE_POLYGON_ENCLOSURE;
			ConvexHull operator()(const Element* elements, size_t count) const;
		};
		inline Wrap<ConvexHull>::CirclePolygonEnclosure Wrap<ConvexHull>::CIRCLE_POLYGON_ENCLOSURE;

		template<size_t K>
		struct Wrap<KDOP<K>>
		{
			KDOP<K> operator()(const Element* elements, size_t count) const
			{
				KDOP<K> kdop;
				kdop.fill_invalid();
				for (size_t i = 0; i < count; ++i)
				{
					std::visit([&kdop](auto&& element) {
						for (size_t j = 0; j < K; ++j)
						{
							std::pair<float, float> interval = element.projection_interval(KDOP<K>::uniform_axis(j));
							kdop.set_minimum(j, std::min(kdop.get_minimum(j), interval.first));
							kdop.set_maximum(j, std::max(kdop.get_maximum(j), interval.second));
						}
						}, elements[i]);
				}
				return kdop;
			}
		};
	}

	template<size_t K, std::array<UnitVector2D, K> Axes>
	struct CustomKDOPShape
	{
		CustomKDOP kdop;
		operator const CustomKDOP& () const { return kdop; }
		operator CustomKDOP& () { return kdop; }
	};

	namespace internal
	{
		template<size_t K, std::array<UnitVector2D, K> Axes>
		struct Wrap<CustomKDOPShape<K, Axes>>
		{
			CustomKDOPShape<K, Axes> operator()(const Element* elements, size_t count) const
			{
				std::vector<float> minima(K, nmax<float>());
				std::vector<float> maxima(K, -nmax<float>());
				for (size_t i = 0; i < count; ++i)
				{
					std::visit([&minima, &maxima](auto&& element) {
						for (size_t j = 0; j < K; ++j)
						{
							std::pair<float, float> interval = element.projection_interval(Axes[j]);
							minima[j] = std::min(minima[j], interval.first);
							maxima[j] = std::max(maxima[j], interval.second);
						}
						}, elements[i]);
				}
				std::vector<UnitVector2D> axes;
				axes.insert(axes.end(), Axes.begin(), Axes.end());
				return { .kdop = CustomKDOP(std::move(axes), std::move(minima), std::move(maxima)) };
			}
		};

		extern glm::vec2 midpoint(const Element& element);

		struct MidpointXY
		{
			bool operator()(const Element& lhs, const Element& rhs) const
			{
				glm::vec2 lm = midpoint(lhs);
				glm::vec2 rm = midpoint(rhs);
				return lm.x < rm.x || (lm.x == rm.x && lm.y <= rm.y);
			}
		};

		struct MidpointYX
		{
			bool operator()(const Element& lhs, const Element& rhs) const
			{
				glm::vec2 lm = midpoint(lhs);
				glm::vec2 rm = midpoint(rhs);
				return lm.y < rm.y || (lm.y == rm.y && lm.x <= rm.x);
			}
		};

		template<bool XY, bool MinX, bool MinY>
		struct ByBounds
		{
			bool operator()(const Element& lhs, const Element& rhs) const
			{
				float(*bx)(const Element& e) = nullptr;
				if constexpr (MinX)
					bx = [](const Element& e) -> float { return std::visit([](auto&& e) -> float { return e.projection_interval(UnitVector2D::LEFT).second; }, e); };
				else
					bx = [](const Element& e) -> float { return std::visit([](auto&& e) -> float { return e.projection_interval(UnitVector2D::RIGHT).second; }, e); };
				
				float(*by)(const Element& e) = nullptr;
				if constexpr (MinY)
					by = [](const Element& e) -> float { return std::visit([](auto&& e) -> float { return e.projection_interval(UnitVector2D::DOWN).second; }, e); };
				else
					by = [](const Element& e) -> float { return std::visit([](auto&& e) -> float { return e.projection_interval(UnitVector2D::UP).second; }, e); };

				if constexpr (XY)
				{
					float lx = bx(lhs);
					float rx = bx(rhs);
					if (lx < rx)
						return true;
					if (lx > rx)
						return false;
					return by(lhs) < by(rhs);
				}
				else
				{
					float ly = by(lhs);
					float ry = by(rhs);
					if (ly < ry)
						return true;
					if (ly > ry)
						return false;
					return bx(lhs) < bx(rhs);
				}
			}
		};
	}

	template<typename Shape>
	class BVH
	{
		struct Node
		{
			std::optional<Shape> shape;
			size_t start_index;
			std::unique_ptr<Node> left, right;

			Node(const Element* elements, size_t start_index, size_t count)
				: start_index(start_index)
			{
				OLY_ASSERT(count > 0);
				if (count > 1)
				{
					shape.emplace(internal::Wrap<Shape>{}(elements + start_index, count));
					left = std::make_unique<Node>(elements, start_index, (count + 1) / 2);
					right = std::make_unique<Node>(elements, start_index + (count + 1) / 2, count / 2);
				}
			}

			Node(const Node& other)
				: shape(other.shape), start_index(other.start_index), left(other.left ? std::make_unique<Node>(*other.left) : nullptr), right(other.right ? std::make_unique<Node>(*other.right) : nullptr)
			{}

			bool is_leaf() const { return !shape.has_value(); }
		};

		mutable Node _root;
		mutable std::vector<Element> elements;
		mutable bool dirty = true;

	public:
		enum class Heuristic
		{
			NONE,
			MIDPOINT_XY,
			MIDPOINT_YX,
			MIN_X_MIN_Y,
			MIN_X_MAX_Y,
			MAX_X_MIN_Y,
			MAX_X_MAX_Y,
			MIN_Y_MIN_X,
			MIN_Y_MAX_X,
			MAX_Y_MIN_X,
			MAX_Y_MAX_X,
		};

		Mask mask = 1;
		Layer layer = 1;

	private:
		Heuristic heuristic = Heuristic::MIDPOINT_XY;

	public:
		BVH() = default;
		BVH(const std::vector<Element>& elements) : elements(elements) {}
		BVH(std::vector<Element>&& elements) : elements(std::move(elements)) {}

		const std::vector<Element>& get_elements() const { return elements; }
		std::vector<Element>& set_elements() { dirty = true; return elements; }

		Heuristic get_heuristic() const { return heuristic; }
		void set_heuristic(Heuristic heuristic) { dirty = true; this->heuristic = heuristic; }

	private:
		const Node& root() const
		{
			if (dirty)
			{
				dirty = false;
				if (heuristic == Heuristic::MIDPOINT_XY)
					std::sort(elements.begin(), elements.end(), internal::MidpointXY{});
				else if (heuristic == Heuristic::MIDPOINT_YX)
					std::sort(elements.begin(), elements.end(), internal::MidpointYX{});
				else if (heuristic == Heuristic::MIN_X_MIN_Y)
					std::sort(elements.begin(), elements.end(), internal::ByBounds<true, true, true>{});
				else if (heuristic == Heuristic::MIN_X_MAX_Y)
					std::sort(elements.begin(), elements.end(), internal::ByBounds<true, true, false>{});
				else if (heuristic == Heuristic::MAX_X_MIN_Y)
					std::sort(elements.begin(), elements.end(), internal::ByBounds<true, false, true>{});
				else if (heuristic == Heuristic::MAX_X_MAX_Y)
					std::sort(elements.begin(), elements.end(), internal::ByBounds<true, false, false>{});
				else if (heuristic == Heuristic::MIN_Y_MIN_X)
					std::sort(elements.begin(), elements.end(), internal::ByBounds<false, true, true>{});
				else if (heuristic == Heuristic::MIN_Y_MAX_X)
					std::sort(elements.begin(), elements.end(), internal::ByBounds<false, false, true>{});
				else if (heuristic == Heuristic::MAX_Y_MIN_X)
					std::sort(elements.begin(), elements.end(), internal::ByBounds<false, true, false>{});
				else if (heuristic == Heuristic::MAX_Y_MAX_X)
					std::sort(elements.begin(), elements.end(), internal::ByBounds<false, false, false>{});
				_root = Node(elements.data(), 0, elements.size());
			}
			return _root;
		}

	public:
		OverlapResult point_hits(glm::vec2 test) const { return point_hits(root(), elements, test); }
		OverlapResult ray_hits(const Ray& ray) const { return ray_hits(root(), elements, ray); }
		RaycastResult raycast(const Ray& ray) const { return raycast(root(), elements, ray); }
		
		template<typename Other>
		OverlapResult raw_overlaps(const Other& c) const { return overlaps(root(), elements, c); }
		template<typename S>
		OverlapResult raw_overlaps(const BVH<S>& bvh) const { return overlaps(root(), elements, bvh.root(), bvh.elements.data()); }

		template<typename Other>
		CollisionResult raw_collides(const Other& c) const { return collides(root(), elements, c); }
		template<typename S>
		CollisionResult raw_collides(const BVH<S>& bvh) const { return collides(root(), elements, bvh.root()); }

		template<typename Other>
		ContactResult raw_contacts(const Other& c) const { return contacts(root(), elements, c); }
		template<typename S>
		ContactResult raw_contacts(const BVH<S>& bvh) const { return contacts(root(), elements, bvh.root()); }

	private:
		static OverlapResult point_hits(const Node& node, const Element* elements, glm::vec2 test)
		{
			if (node.is_leaf())
				return col2d::point_hits(elements[node.start_index], test);
			else if (!col2d::point_hits(node.shape.value(), test))
				return false;
			else
				return point_hits(*node.left, test) || point_hits(*node.right, test);
		}

		static OverlapResult ray_hits(const Node& node, const Element* elements, const Ray& ray)
		{
			if (node.is_leaf())
				return col2d::ray_hits(elements[node.start_index], ray);
			else if (!col2d::ray_hits(node.shape.value(), ray))
				return false;
			else
				return ray_hits(*node.left, ray) || ray_hits(*node.right, ray);
		}

		static RaycastResult raycast(const Node& node, const Element* elements, const Ray& ray)
		{
			if (node.is_leaf())
				return col2d::raycast(elements[node.start_index], ray);
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
		static OverlapResult overlaps(const Node& my_node, const Element* my_elements, const BVH<OtherShape>::Node& other_node, const Element* other_elements)
		{
			if (my_node.is_leaf())
			{
				if (other_node.is_leaf())
					return col2d::overlaps(my_elements[my_node.start_index], other_elements[other_node.start_index]);
				else
					return BVH<OtherShape>::overlaps(other_node, other_elements, my_elements[my_node.start_index]);
			}
			else
			{
				if (other_node.is_leaf())
					return overlaps(my_node, my_elements, other_elements[other_node.start_index]);
				else
				{
					if (!col2d::overlaps(my_node.shape.value(), other_node.shape.value()))
						return false;
					else
						return overlaps(*my_node.left, my_elements, *other_node.left, other_elements)
							|| overlaps(*my_node.left, my_elements, *other_node.right, other_elements)
							|| overlaps(*my_node.right, my_elements, *other_node.left, other_elements)
							|| overlaps(*my_node.right, my_elements, *other_node.right, other_elements);
				}
			}
		}

		template<typename Other>
		static OverlapResult overlaps(const Node& node, const Element* elements, const Other& c)
		{
			if (node.is_leaf())
				return overlaps(elements[node.start_index], c);
			else if (!col2d::overlaps(node.shape.value(), c))
				return false;
			else
				return overlaps(*node.left, c) || overlaps(*node.right, c);
		}

		template<typename OtherShape>
		static CollisionResult collides(const Node& my_node, const Element* my_elements, const BVH<OtherShape>::Node& other_node, const Element* other_elements)
		{
			if (my_node.is_leaf())
			{
				if (other_node.is_leaf())
					return collides(my_elements[my_node.start_index], other_elements[other_node.start_index]);
				else
					return BVH<OtherShape>::collides(other_node, other_elements, my_elements[my_node.start_index]).invert();
			}
			else
			{
				if (other_node.is_leaf())
					return collides(my_node, my_elements, other_elements[other_node.start_index]);
				else
				{
					if (!col2d::overlaps(my_node.shape.value(), other_node.shape.value()))
						return false;
					else
						return greedy_collision({ collides(*my_node.left,  my_elements, *other_node.left, other_elements), collides(*my_node.left,  my_elements, *other_node.right, other_elements),
												  collides(*my_node.right, my_elements, *other_node.left, other_elements), collides(*my_node.right, my_elements, *other_node.right, other_elements) });
				}
			}
		}

		template<typename Other>
		static CollisionResult collides(const Node& node, const Other& c)
		{
			if (node.is_leaf())
				return collides(elements[node.start_index], c);
			else if (!col2d::overlaps(node.shape.value(), c))
				return { .overlap = false };
			else
				return greedy_collision({ collides(*node.left, c), collides(*node.right, c) });
		}

		template<typename OtherShape>
		static ContactResult contacts(const Node& my_node, const Element* my_elements, const BVH<OtherShape>::Node& other_node, const Element* other_elements)
		{
			if (my_node.is_leaf())
			{
				if (other_node.is_leaf())
					return contacts(my_elements[my_node.start_index], other_elements[other_node.start_index]);
				else
					return BVH<OtherShape>::contacts(other_node, other_elements, my_elements[my_node.start_index]).invert();
			}
			else
			{
				if (other_node.is_leaf())
					return contacts(my_node, my_elements, other_elements[other_node.start_index]);
				else
					return greedy_contact({ contacts(*my_node.left,  my_elements, *other_node.left, other_elements), contacts(*my_node.left,  my_elements, *other_node.right, other_elements),
											contacts(*my_node.right, my_elements, *other_node.left, other_elements), contacts(*my_node.right, my_elements, *other_node.right, other_elements) });
			}
		}

		template<typename Other>
		static ContactResult contacts(const Node& node, const Element* elements, const Other& c)
		{
			if (node.is_leaf())
				return contacts(elements[node.start_index], c);
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
		std::vector<Element> local_elements;

		const BVH<Shape>& bvh() const
		{
			if (local_dirty)
			{
				local_dirty = false;
				const glm::mat3 m = transformer.global();
				std::vector<Element>& global_elements = _bvh.set_elements();
				global_elements.resize(local_elements.size());
				for (size_t i = 0; i < local_elements.size(); ++i)
					global_elements[i] = transform_element(local_elements[i], m);
			}
			return _bvh;
		}

	public:
		Transformer2D transformer;

		TBVH() = default;
		TBVH(const std::vector<Element>& elements) : local_elements(elements) {}
		TBVH(std::vector<Element>&& elements) : local_elements(std::move(elements)) {}
		TBVH(const BVH<Shape>& bvh) : local_elements(bvh.get_elements()) { _bvh.mask = bvh.mask; _bvh.layer = bvh.layer; _bvh.set_heuristic(bvh.get_heuristic()); }
		TBVH(BVH<Shape>&& bvh) : local_elements(std::move(bvh.set_elements())) { _bvh.mask = bvh.mask; _bvh.layer = bvh.layer; _bvh.set_heuristic(bvh.get_heuristic()); }

		const Transform2D& get_local() const { return transformer.get_local(); }
		Transform2D& set_local() { local_dirty = true; return transformer.get_local(); }

		const std::vector<Element>& get_elements() const { return local_elements; }
		std::vector<Element>& set_elements() { local_dirty = true; return local_elements; }

		Mask mask() const { return _bvh.mask; }
		Mask& mask() { return _bvh.mask; }
		Layer layer() const { return _bvh.layer; }
		Layer& layer() { return _bvh.layer; }

		OverlapResult point_hits(glm::vec2 test) const { return bvh().point_hits(test); }
		OverlapResult ray_hits(const Ray& ray) const { return bvh().ray_hits(ray); }
		RaycastResult raycast(const Ray& ray) const { return bvh().raycast(ray); }

		template<typename Other>
		OverlapResult raw_overlaps(const Other& c) const { return bvh().raw_overlaps(c); }
		template<typename S>
		OverlapResult raw_overlaps(const BVH<S>& c) const { return bvh().raw_overlaps(c); }
		template<typename S>
		OverlapResult raw_overlaps(const TBVH<S>& c) const { return bvh().raw_overlaps(c.bvh()); }

		template<typename Other>
		CollisionResult raw_collides(const Other& c) const { return bvh().raw_collides(c); }
		template<typename S>
		CollisionResult raw_collides(const BVH<S>& c) const { return bvh().raw_collides(c); }
		template<typename S>
		CollisionResult raw_collides(const TBVH<S>& c) const { return bvh().raw_collides(c.bvh()); }

		template<typename Other>
		ContactResult raw_contacts(const Other& c) const { return bvh().raw_contacts(c); }
		template<typename S>
		ContactResult raw_contacts(const BVH<S>& c) const { return bvh().raw_contacts(c); }
		template<typename S>
		ContactResult raw_contacts(const TBVH<S>& c) const { return bvh().raw_contacts(c.bvh()); }
	};
}
