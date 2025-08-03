#pragma once

#include "physics/collision/Tolerance.h"
#include "physics/collision/elements/Element.h"
#include "physics/collision/methods/Collide.h"
#include "physics/collision/methods/KDOPCollide.h"
#include "physics/collision/objects/Primitive.h"
#include "physics/collision/objects/Compound.h"

#include "core/base/Transforms.h"
#include "core/base/TransformerExposure.h"
#include "core/base/Assert.h"
#include "core/types/DeferredFalse.h"
#include "core/math/Solvers.h"
#include "core/containers/DoubleBuffer.h"

#include <algorithm>

namespace oly::col2d
{
	namespace internal
	{
		template<typename Shape>
		struct Wrap
		{
			Shape operator()(const Element* elements, size_t count) const { static_assert(deferred_false<Shape>, "oly::col2d::internal::wrap() not defined for the provided Shape"); }
		};

		template<>
		struct Wrap<AABB>
		{
			AABB operator()(const Element* elements, size_t count) const;
			AABB operator()(const Element& element) const;
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
					for (size_t j = 0; j < K; ++j)
					{
						fpair interval = elements[i].projection_interval(KDOP<K>::uniform_axis(j));
						kdop.set_minimum(j, std::min(kdop.get_minimum(j), interval.first));
						kdop.set_maximum(j, std::max(kdop.get_maximum(j), interval.second));
					}
				}
				return kdop;
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
					bx = [](const Element& e) -> float { return e.projection_max(UnitVector2D::LEFT); };
				else
					bx = [](const Element& e) -> float { return e.projection_max(UnitVector2D::RIGHT); };
				
				float(*by)(const Element& e) = nullptr;
				if constexpr (MinY)
					by = [](const Element& e) -> float { return e.projection_max(UnitVector2D::DOWN); };
				else
					by = [](const Element& e) -> float { return e.projection_min(UnitVector2D::UP); };

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

	template<typename Shape>
	class BVH
	{
		template<typename>
		friend class BVH;

		struct Node
		{
			std::optional<Shape> shape;
			size_t start_index = 0, count = 0;
			std::unique_ptr<Node> left, right;

			Node() = default;

			Node(const Element* elements, size_t start_index, size_t count)
				: start_index(start_index), count(count)
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
				: shape(other.shape), start_index(other.start_index), count(other.count), left(other.left ? std::make_unique<Node>(*other.left) : nullptr),
				right(other.right ? std::make_unique<Node>(*other.right) : nullptr)
			{}

			Node(Node&& other) noexcept
				: shape(std::move(other.shape)), start_index(other.start_index), count(other.count), left(std::move(other.left)), right(std::move(other.right))
			{}

			Node& operator=(const Node& other)
			{
				if (this != &other)
				{
					shape = other.shape;
					start_index = other.start_index;
					count = other.count;
					left = other.left ? std::make_unique<Node>(*other.left) : nullptr;
					right = other.right ? std::make_unique<Node>(*other.right) : nullptr;
				}
				return *this;
			}

			Node& operator=(Node&& other) noexcept
			{
				if (this != &other)
				{
					shape = std::move(other.shape);
					start_index = other.start_index;
					count = other.count;
					left = std::move(other.left);
					right = std::move(other.right);
				}
				return *this;
			}

			bool is_leaf() const { return !shape.has_value(); }
		};

		mutable Node _root;
		mutable std::vector<Element> elements;
		mutable bool dirty = true;
		Heuristic heuristic = Heuristic::MIDPOINT_XY;

	public:
		Mask mask = 0;
		Layer layer = 0;

		CompoundPerfParameters perf = {};

		BVH() = default;
		explicit BVH(const std::vector<Element>& elements) : elements(elements) {}
		explicit BVH(std::vector<Element>&& elements) : elements(std::move(elements)) {}

		const std::vector<Element>& get_elements() const { return elements; }
		std::vector<Element>& set_elements() { dirty = true; return elements; }

		Heuristic get_heuristic() const { return heuristic; }
		void set_heuristic(Heuristic heuristic) { dirty = true; this->heuristic = heuristic; }

		size_t get_depth_cap() const { elements.empty() ? 0 : (size_t)glm::ceil(glm::log2((float)elements.size())); }

		const Shape& root_shape() const { return *root().shape; }

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
		std::vector<const Element*> build_layer(size_t at_depth) const
		{
			std::vector<const Element*> layer;
			DoubleBuffer<const Node*> nodes;
			nodes.back.push_back(&root());

			for (size_t i = 0; i < at_depth; ++i)
			{
				nodes.swap().back.clear();
				if (nodes.front.empty())
					return layer;
				for (const Node* node : nodes.front)
				{
					if (node->is_leaf())
						layer.push_back(elements[node->start_index]);
					else
					{
						nodes.back.push_back(node->left.get());
						nodes.back.push_back(node->right.get());
					}
				}
			}

			nodes.swap();
			for (const Node* node : nodes.front)
			{
				if (node->is_leaf())
					layer.push_back(elements[node->start_index]);
				else
					layer.push_back(*node->shape);
			}
			return layer;
		}

		float projection_max(const UnitVector2D& axis) const
		{
			float proj_max = -nmax<float>();
			for (const Element& element : elements)
				proj_max = std::max(proj_max, element.projection_max(axis));
			return proj_max;
		}

		float projection_min(const UnitVector2D& axis) const
		{
			float proj_min = nmax<float>();
			for (const Element& element : elements)
				proj_min = std::max(proj_min, element.projection_min(axis));
			return proj_min;
		}

		OverlapResult point_hits(glm::vec2 test) const { return point_hits(root(), elements.data(), test); }
		OverlapResult ray_hits(Ray ray) const { return ray_hits(root(), elements.data(), ray); }
		RaycastResult raycast(Ray ray) const { return raycast(root(), elements.data(), ray); }
		
		OverlapResult raw_overlaps(const Element& e) const { return overlaps(root(), elements.data(), e); }
		template<typename Other>
		OverlapResult raw_overlaps(const Other& c) const { return overlaps(root(), elements.data(), c); }
		template<typename S>
		OverlapResult raw_overlaps(const BVH<S>& bvh) const { return overlaps<S>(root(), elements.data(), bvh.root(), bvh.elements.data()); }

		CollisionResult raw_collides(const Element& e) const
			{ return raw_overlaps(e) ? compound_collision(elements.data(), elements.size(), e, perf) : CollisionResult{ .overlap = false }; }
		CollisionResult raw_collides(const Primitive& c) const
			{ return raw_overlaps(c) ? compound_collision(elements.data(), elements.size(), c.element, perf) : CollisionResult{ .overlap = false }; }
		CollisionResult raw_collides(const TPrimitive& c) const
			{ return raw_overlaps(c) ? compound_collision(elements.data(), elements.size(), c.get_baked(), perf) : CollisionResult{ .overlap = false }; }
		CollisionResult raw_collides(const Compound& c) const
		{
			return raw_overlaps(c)
				? compound_collision(elements.data(), elements.size(), c.elements.data(), c.elements.size(), CompoundPerfParameters::greedy(perf, c.perf))
				: CollisionResult{ .overlap = false };
		}
		CollisionResult raw_collides(const TCompound& c) const
		{
			return raw_overlaps(c)
				? compound_collision(elements.data(), elements.size(), c.get_baked().data(), c.get_baked().size(), CompoundPerfParameters::greedy(perf, c.perf()))
				: CollisionResult{ .overlap = false };
		}

		template<typename S>
		CollisionResult raw_collides(const BVH<S>& bvh) const
		{
			return raw_overlaps(bvh)
				? compound_collision(elements.data(), elements.size(), bvh.elements.data(), bvh.elements.size(), CompoundPerfParameters::greedy(perf, bvh.perf))
				: CollisionResult{.overlap = false};
		}

		ContactResult raw_contacts(const Element& e) const
			{ return raw_overlaps(e) ? compound_contact(elements.data(), elements.size(), e, perf) : ContactResult{ .overlap = false }; }
		ContactResult raw_contacts(const Primitive& c) const
			{ return raw_overlaps(c) ? compound_contact(elements.data(), elements.size(), c.element, perf) : ContactResult{ .overlap = false }; }
		ContactResult raw_contacts(const TPrimitive& c) const
			{ return raw_overlaps(c) ? compound_contact(elements.data(), elements.size(), c.get_baked(), perf) : ContactResult{ .overlap = false }; }
		ContactResult raw_contacts(const Compound& c) const
		{
			return raw_overlaps(c)
				? compound_contact(elements.data(), elements.size(), c.elements.data(), c.elements.size(), CompoundPerfParameters::greedy(perf, c.perf))
				: ContactResult{ .overlap = false };
		}
		ContactResult raw_contacts(const TCompound& c) const
		{
			return raw_overlaps(c)
				? compound_contact(elements.data(), elements.size(), c.get_baked().data(), c.get_baked().size(), CompoundPerfParameters::greedy(perf, c.perf()))
				: ContactResult{ .overlap = false };
		}

		template<typename S>
		ContactResult raw_contacts(const BVH<S>& bvh) const
		{
			return raw_overlaps(bvh)
				? compound_contact(elements.data(), elements.size(), bvh.elements.data(), bvh.elements.size(), CompoundPerfParameters::greedy(perf, bvh.perf))
				: ContactResult{ .overlap = false };
		}

	private:
		static OverlapResult point_hits(const Node& node, const Element* elements, glm::vec2 test)
		{
			if (node.is_leaf())
				return col2d::point_hits(elements[node.start_index], test);
			else if (!col2d::point_hits(node.shape.value(), test))
				return false;
			else
				return point_hits(*node.left, elements, test) || point_hits(*node.right, elements, test);
		}

		static OverlapResult ray_hits(const Node& node, const Element* elements, Ray ray)
		{
			if (node.is_leaf())
				return col2d::ray_hits(elements[node.start_index], ray);
			else if (!col2d::ray_hits(node.shape.value(), ray))
				return false;
			else
				return ray_hits(*node.left, elements, ray) || ray_hits(*node.right, elements, ray);
		}

		static RaycastResult raycast(const Node& node, const Element* elements, Ray ray)
		{
			if (node.is_leaf())
				return col2d::raycast(elements[node.start_index], ray);
			else if (!col2d::ray_hits(node.shape.value(), ray))
				return { .hit = RaycastResult::Hit::NO_HIT };
			else
			{
				RaycastResult left_result = raycast(*node.left, elements, ray);
				if (left_result.hit == RaycastResult::Hit::EMBEDDED_ORIGIN)
					return left_result;
				RaycastResult right_result = raycast(*node.right, elements, ray);
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
		static OverlapResult overlaps(const Node& my_node, const Element* my_elements, const typename BVH<OtherShape>::Node& other_node, const Element* other_elements)
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
						return overlaps<OtherShape>(*my_node.left, my_elements, *other_node.left, other_elements)
							|| overlaps<OtherShape>(*my_node.left, my_elements, *other_node.right, other_elements)
							|| overlaps<OtherShape>(*my_node.right, my_elements, *other_node.left, other_elements)
							|| overlaps<OtherShape>(*my_node.right, my_elements, *other_node.right, other_elements);
				}
			}
		}

		template<typename Other>
		static OverlapResult overlaps(const Node& node, const Element* elements, const Other& c)
		{
			if (node.is_leaf())
				return col2d::overlaps(elements[node.start_index], c);
			else if (!col2d::overlaps(node.shape.value(), c))
				return false;
			else
				return overlaps(*node.left, elements, c) || overlaps(*node.right, elements, c);
		}
	};

	namespace internal { struct LUT; };

	template<typename Shape>
	class TBVH
	{
		template<typename>
		friend class TBVH;

		friend struct internal::LUT;

		Transformer2D transformer;
		mutable BVH<Shape> _bvh;
		mutable bool local_dirty = true;
		std::vector<Element> local_elements;

		const BVH<Shape>& bvh() const
		{
			if (local_dirty || transformer.flush())
			{
				local_dirty = false;
				const glm::mat3 m = transformer.global();
				std::vector<Element>& global_elements = _bvh.set_elements();
				global_elements.resize(local_elements.size());
				for (size_t i = 0; i < local_elements.size(); ++i)
					global_elements[i] = local_elements[i].transformed(m);
			}
			return _bvh;
		}

	public:
		TBVH() = default;
		explicit TBVH(const std::vector<Element>& elements) : local_elements(elements) {}
		explicit TBVH(std::vector<Element>&& elements) : local_elements(std::move(elements)) {}
		explicit TBVH(const BVH<Shape>& bvh) : local_elements(bvh.get_elements()) { _bvh.mask = bvh.mask; _bvh.layer = bvh.layer; _bvh.set_heuristic(bvh.get_heuristic()); }
		explicit TBVH(BVH<Shape>&& bvh) : local_elements(std::move(bvh.set_elements())) { _bvh.mask = bvh.mask; _bvh.layer = bvh.layer; _bvh.set_heuristic(bvh.get_heuristic()); }

		const Transform2D& get_local() const { return transformer.get_local(); }
		Transform2D& set_local() { return transformer.set_local(); }
		bool is_dirty() const { return local_dirty || transformer.dirty(); }

		Transformer2DConstExposure get_transformer() const { return transformer; }
		Transformer2DExposure<TExposureParams{ .local = exposure::local::FULL, .chain = exposure::chain::FULL, .modifier = exposure::modifier::FULL }> set_transformer() { return transformer; }

		const std::vector<Element>& get_elements() const { return local_elements; }
		std::vector<Element>& set_elements() { local_dirty = true; return local_elements; }

		const std::vector<Element>& get_baked() const { return bvh().get_elements(); }

		const Shape& root_shape() const { return bvh().root_shape(); }

		Mask mask() const { return _bvh.mask; }
		Mask& mask() { return _bvh.mask; }
		Layer layer() const { return _bvh.layer; }
		Layer& layer() { return _bvh.layer; }

		const CompoundPerfParameters& perf() const { return _bvh.perf; }
		CompoundPerfParameters& perf() { return _bvh.perf; }
		
		Heuristic get_heuristic() const { return _bvh.get_heuristic(); }
		void set_heuristic(Heuristic heuristic) { _bvh.set_heuristic(heuristic); }
		size_t get_depth_cap() const { return local_elements.empty() ? 0 : (size_t)glm::ceil(glm::log2((float)local_elements.size())); }

		std::vector<const Element*> build_layer(size_t at_depth) const { return bvh().build_layer(at_depth); }

		float projection_max(const UnitVector2D& axis) const
		{
			float proj_max = -nmax<float>();
			for (const Element& element : bvh().get_elements())
				proj_max = std::max(proj_max, element.projection_max(axis));
			return proj_max;
		}

		float projection_min(const UnitVector2D& axis) const
		{
			float proj_min = nmax<float>();
			for (const Element& element : bvh().get_elements())
				proj_min = std::min(proj_min, element.projection_min(axis));
			return proj_min;
		}

		OverlapResult point_hits(glm::vec2 test) const { return bvh().point_hits(test); }
		OverlapResult ray_hits(Ray ray) const { return bvh().ray_hits(ray); }
		RaycastResult raycast(Ray ray) const { return bvh().raycast(ray); }

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
