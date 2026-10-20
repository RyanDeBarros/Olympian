#pragma once

#include "core/base/Assert.h"
#include "core/base/TransformerExposure.h"
#include "core/base/Transforms.h"

#include "core/math/Solvers.h"

#include "physics/collision/Tolerance.h"

#include "physics/collision/elements/Element.h"

#include "physics/collision/methods/Collide.h"
#include "physics/collision/methods/KDOPCollide.h"

#include "physics/collision/objects/Primitive.h"
#include "physics/collision/objects/Compound.h"

#include <imp/dependent_false.hpp>
#include <imp/swapper.hpp>

namespace oly::col2d
{
	namespace internal
	{
		template<typename Shape>
		struct Wrap
		{
			Shape operator()(const Element* elements, size_t count) const
            {
                static_assert(imp::dependent_false_v<Shape>, "oly::col2d::internal::Wrap::operator()() not defined for the provided Shape");
            }
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
				size_t degree_ = 8;
				float overfitting_multiplier_ = 1.f;

			public:
                CirclePolygonEnclosure(size_t degree = 8);

			public:
                size_t get_degree() const;
                void set_degree(size_t d);
                glm::vec2 get_point(const Circle& c, size_t i) const;

            private:
                void set_mult();
			};

			static CirclePolygonEnclosure kCirclePolygonEnclosure;

			ConvexHull operator()(const Element* elements, size_t count) const;
		};
		inline Wrap<ConvexHull>::CirclePolygonEnclosure Wrap<ConvexHull>::kCirclePolygonEnclosure;

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
            bool operator()(const Element& lhs, const Element& rhs) const;
		};

		struct MidpointYX
		{
            bool operator()(const Element& lhs, const Element& rhs) const;
		};

		template<bool XY, bool MinX, bool MinY>
		struct ByBounds
		{
			bool operator()(const Element& lhs, const Element& rhs) const
			{
				float(*bx)(const Element& e) = nullptr;
				if constexpr (MinX)
					bx = [](const Element& e) -> float { return e.projection_max(UnitVector2D::kLeft); };
				else
					bx = [](const Element& e) -> float { return e.projection_max(UnitVector2D::kRight); };
				
				float(*by)(const Element& e) = nullptr;
				if constexpr (MinY)
					by = [](const Element& e) -> float { return e.projection_max(UnitVector2D::kDown); };
				else
					by = [](const Element& e) -> float { return e.projection_min(UnitVector2D::kUp); };

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
		None,
		MidpointXY,
		MidpointYX,
		MinXMinY,
		MinXMaxY,
		MaxXMinY,
		MaxXMaxY,
		MinYMinX,
		MinYMaxX,
		MaxYMinX,
		MaxYMaxX,
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
				: start_index(start_index)
                , count(count)
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
				: shape(other.shape)
                , start_index(other.start_index)
                , count(other.count)
                , left(other.left ? std::make_unique<Node>(*other.left) : nullptr)
                , right(other.right ? std::make_unique<Node>(*other.right) : nullptr)
			{
            }

			Node(Node&& other) noexcept
				: shape(std::move(other.shape))
                , start_index(other.start_index)
                , count(other.count)
                , left(std::move(other.left))
                , right(std::move(other.right))
			{
            }

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

			bool is_leaf() const
            {
                return !shape.has_value();
            }
		};

		mutable Node root_;
		mutable std::vector<Element> elements_;
		mutable bool dirty_ = true;
		Heuristic heuristic_ = Heuristic::MidpointXY;

	public:
		Mask mask = 0;
		Layer layer = 0;

		CompoundPerfParameters perf = {};

		BVH() = default;

        explicit BVH(std::vector<Element> elements)
            : elements_(std::move(elements))
        {
        }

		const std::vector<Element>& get_elements() const
        {
            return elements_;
        }

		std::vector<Element>& set_elements()
        {
            dirty_ = true;
            return elements_;
        }

		template<internal::Elem_check Shape>
		const Shape& element_as(size_t i) const
		{
			return *get_elements()[i].variant().get<const Shape*>();
		}

		template<internal::Elem_check Shape>
		Shape& element_as(size_t i)
		{
			return *set_elements()[i].variant().get<Shape*>();
		}

		Heuristic get_heuristic() const
        {
            return heuristic_;
        }

		void set_heuristic(Heuristic heuristic)
        {
            dirty_ = true;
            this->heuristic_ = heuristic;
        }

		size_t get_depth_cap() const
        {
            return elements_.empty() ? 0 : (size_t)glm::ceil(glm::log2((float)elements_.size()));
        }

		const Shape& root_shape() const
        {
            return *root().shape;
        }

		std::vector<const Element*> build_layer(size_t at_depth) const
		{
			std::vector<const Element*> layer;
			imp::swapper<std::vector<const Node*>> nodes;
			nodes.back.push_back(&root());

			for (size_t i = 0; i < at_depth; ++i)
			{
				nodes.swap().back.clear();
				if (nodes.front.empty())
					return layer;
				for (const Node* node : nodes.front)
				{
					if (node->is_leaf())
						layer.push_back(elements_[node->start_index]);
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
					layer.push_back(elements_[node->start_index]);
				else
					layer.push_back(*node->shape);
			}
			return layer;
		}

		float projection_max(const UnitVector2D& axis) const
		{
			float proj_max = -nmax<float>();
			for (const Element& element : elements_)
				proj_max = std::max(proj_max, element.projection_max(axis));
			return proj_max;
		}

		float projection_min(const UnitVector2D& axis) const
		{
			float proj_min = nmax<float>();
			for (const Element& element : elements_)
				proj_min = std::max(proj_min, element.projection_min(axis));
			return proj_min;
		}

		OverlapResult point_hits(glm::vec2 test) const
        {
            return point_hits(root(), elements_.data(), test);
        }

		OverlapResult ray_hits(Ray ray) const
        {
            return ray_hits(root(), elements_.data(), ray);
        }
		
        RaycastResult raycast(Ray ray) const
        {
            return raycast(root(), elements_.data(), ray);
        }
		
		OverlapResult raw_overlaps(const Element& e) const
        {
            return overlaps(root(), elements_.data(), e);
        }
		
        template<typename Other>
		OverlapResult raw_overlaps(const Other& c) const
        {
            return overlaps(root(), elements_.data(), c);
        }
		
        template<typename S>
		OverlapResult raw_overlaps(const BVH<S>& bvh) const
        {
            return overlaps<S>(root(), elements_.data(), bvh.root(), bvh.elements_.data());
        }

		CollisionResult raw_collides(const Element& e) const
		{
            return raw_overlaps(e)
                ? compound_collision(elements_.data(), elements_.size(), e, perf)
                : CollisionResult{ .overlap = false };
        }
		
        CollisionResult raw_collides(const Primitive& c) const
		{
            return raw_overlaps(c)
                ? compound_collision(elements_.data(), elements_.size(), c.element, perf)
                : CollisionResult{ .overlap = false };
        }
		
        CollisionResult raw_collides(const TPrimitive& c) const
        {
            return raw_overlaps(c)
                ? compound_collision(elements_.data(), elements_.size(), c.get_baked(), perf)
                : CollisionResult{ .overlap = false };
        }
		
        CollisionResult raw_collides(const Compound& c) const
		{
			return raw_overlaps(c)
				? compound_collision(
                    elements_.data(),
                    elements_.size(),
                    c.elements.data(),
                    c.elements.size(),
                    CompoundPerfParameters::greedy(perf, c.perf))
				: CollisionResult{ .overlap = false };
		}

		CollisionResult raw_collides(const TCompound& c) const
		{
			return raw_overlaps(c)
				? compound_collision(
                    elements_.data(),
                    elements_.size(),
                    c.get_baked().data(),
                    c.get_baked().size(),
                    CompoundPerfParameters::greedy(perf, c.perf()))
				: CollisionResult{ .overlap = false };
		}

		template<typename S>
		CollisionResult raw_collides(const BVH<S>& bvh) const
		{
			return raw_overlaps(bvh)
				? compound_collision(
                    elements_.data(),
                    elements_.size(),
                    bvh.elements_.data(),
                    bvh.elements_.size(),
                    CompoundPerfParameters::greedy(perf, bvh.perf))
				: CollisionResult{.overlap = false};
		}

		ContactResult raw_contacts(const Element& e) const
		{
            return raw_overlaps(e)
                ? compound_contact(elements_.data(), elements_.size(), e, perf)
                : ContactResult{ .overlap = false };
        }

		ContactResult raw_contacts(const Primitive& c) const
		{
            return raw_overlaps(c)
                ? compound_contact(elements_.data(), elements_.size(), c.element, perf)
                : ContactResult{ .overlap = false };
        }
		
        ContactResult raw_contacts(const TPrimitive& c) const
        {
            return raw_overlaps(c)
                ? compound_contact(elements_.data(), elements_.size(), c.get_baked(), perf)
                : ContactResult{ .overlap = false };
        }
		
		ContactResult raw_contacts(const Compound& c) const
		{
			return raw_overlaps(c)
				? compound_contact(
                    elements_.data(),
                    elements_.size(),
                    c.elements.data(),
                    c.elements.size(),
                    CompoundPerfParameters::greedy(perf, c.perf))
				: ContactResult{ .overlap = false };
		}
		ContactResult raw_contacts(const TCompound& c) const
		{
			return raw_overlaps(c)
				? compound_contact(
                    elements_.data(),
                    elements_.size(),
                    c.get_baked().data(),
                    c.get_baked().size(),
                    CompoundPerfParameters::greedy(perf, c.perf()))
				: ContactResult{ .overlap = false };
		}

		template<typename S>
		ContactResult raw_contacts(const BVH<S>& bvh) const
		{
			return raw_overlaps(bvh)
				? compound_contact(
                    elements_.data(),
                    elements_.size(),
                    bvh.elements_.data(),
                    bvh.elements_.size(),
                    CompoundPerfParameters::greedy(perf, bvh.perf))
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
				return { .hit = RaycastResult::Hit::NoHit };
			else
			{
				RaycastResult left_result = raycast(*node.left, elements, ray);
				if (left_result.hit == RaycastResult::Hit::EmbeddedOrigin)
					return left_result;
				RaycastResult right_result = raycast(*node.right, elements, ray);
				if (right_result.hit == RaycastResult::Hit::EmbeddedOrigin)
					return right_result;
				
				if (left_result.hit == RaycastResult::Hit::NoHit)
				{
					if (right_result.hit == RaycastResult::Hit::NoHit)
						return { .hit = RaycastResult::Hit::NoHit };
					else
						return right_result;
				}
				else
				{
					if (right_result.hit == RaycastResult::Hit::NoHit)
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

        const Node& root() const
        {
            if (dirty_)
            {
                dirty_ = false;

                switch (heuristic_)
                {
                case Heuristic::MidpointXY:
                    std::sort(elements_.begin(), elements_.end(), internal::MidpointXY{});
                    break;
                case Heuristic::MidpointYX:
                    std::sort(elements_.begin(), elements_.end(), internal::MidpointYX{});
                    break;
                case Heuristic::MinXMinY:
                    std::sort(elements_.begin(), elements_.end(), internal::ByBounds<true, true, true>{});
                    break;
                case Heuristic::MinXMaxY:
                    std::sort(elements_.begin(), elements_.end(), internal::ByBounds<true, true, false>{});
                    break;
                case Heuristic::MaxXMinY:
                    std::sort(elements_.begin(), elements_.end(), internal::ByBounds<true, false, true>{});
                    break;
                case Heuristic::MaxXMaxY:
                    std::sort(elements_.begin(), elements_.end(), internal::ByBounds<true, false, false>{});
                    break;
                case Heuristic::MinYMinX:
                    std::sort(elements_.begin(), elements_.end(), internal::ByBounds<false, true, true>{});
                    break;
                case Heuristic::MinYMaxX:
                    std::sort(elements_.begin(), elements_.end(), internal::ByBounds<false, false, true>{});
                    break;
                case Heuristic::MaxYMinX:
                    std::sort(elements_.begin(), elements_.end(), internal::ByBounds<false, true, false>{});
                    break;
                case Heuristic::MaxYMaxX:
                    std::sort(elements_.begin(), elements_.end(), internal::ByBounds<false, false, false>{});
                    break;
                }

                root_ = Node(elements_.data(), 0, elements_.size());
            }

            return root_;
        }
	};

	namespace internal { struct LUT; };

	template<typename Shape>
	class TBVH
	{
		template<typename>
		friend class TBVH;

		friend struct internal::LUT;

		Transformer2D transformer_;
		mutable BVH<Shape> bvh_;
		mutable bool local_dirty_ = true;
		std::vector<Element> local_elements_;

	public:
		TBVH() = default;

        explicit TBVH(std::vector<Element> elements)
            : local_elements_(std::move(elements))
        {
        }
		
        explicit TBVH(const BVH<Shape>& bvh)
            : local_elements_(bvh.get_elements())
        {
            bvh_.mask = bvh.mask;
            bvh_.layer = bvh.layer;
            bvh_.set_heuristic(bvh.get_heuristic());
        }
		
        explicit TBVH(BVH<Shape>&& bvh)
            : local_elements_(std::move(bvh.set_elements()))
        {
            bvh_.mask = bvh.mask;
            bvh_.layer = bvh.layer;
            bvh_.set_heuristic(bvh.get_heuristic());
        }

    public:
		const Transform2D& get_local() const
        {
            return transformer_.get_local();
        }

		Transform2D& set_local()
        {
            return transformer_.set_local();
        }

		bool is_dirty() const
        {
            return local_dirty_ || transformer_.dirty();
        }

		Transformer2DExposure<> get_transformer() const
        {
            return transformer_;
        }
		
        Transformer2DExposure<ex::kFull> set_transformer()
        {
            return transformer_;
        }

		const std::vector<Element>& get_elements() const
        {
            return local_elements_;
        }
		
        std::vector<Element>& set_elements()
        {
            local_dirty_ = true;
            return local_elements_;
        }
		
		template<internal::Elem_check Shape>
		const Shape& element_as(size_t i) const
		{
			return *get_elements()[i].variant().get<const Shape*>();
		}

		template<internal::Elem_check Shape>
		Shape& element_as(size_t i)
		{
			return *set_elements()[i].variant().get<Shape*>();
		}

		const std::vector<Element>& get_baked() const
        {
            return bvh().get_elements();
        }

		const Shape& root_shape() const
        {
            return bvh().root_shape();
        }

		Mask mask() const
        {
            return bvh_.mask;
        }

		Mask& mask()
        {
            return bvh_.mask;
        }
		
        Layer layer() const
        {
            return bvh_.layer;
        }
		
        Layer& layer()
        {
            return bvh_.layer;
        }

		const CompoundPerfParameters& perf() const
        {
            return bvh_.perf;
        }

		CompoundPerfParameters& perf()
        {
            return bvh_.perf;
        }
		
		Heuristic get_heuristic() const
        {
            return bvh_.get_heuristic();
        }

		void set_heuristic(Heuristic heuristic)
        {
            bvh_.set_heuristic(heuristic);
        }
		
        size_t get_depth_cap() const
        {
            return local_elements_.empty() ? 0 : (size_t)glm::ceil(glm::log2((float)local_elements_.size()));
        }

		auto build_layer(size_t at_depth) const
        {
            return bvh().build_layer(at_depth);
        }

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

		OverlapResult point_hits(glm::vec2 test) const
        {
            return bvh().point_hits(test);
        }

        OverlapResult ray_hits(Ray ray) const
        {
            return bvh().ray_hits(ray);
        }
		
        RaycastResult raycast(Ray ray) const
        {
            return bvh().raycast(ray);
        }

		template<typename Other>
		OverlapResult raw_overlaps(const Other& c) const
        {
            return bvh().raw_overlaps(c);
        }
		
        template<typename S>
		OverlapResult raw_overlaps(const BVH<S>& c) const
        {
            return bvh().raw_overlaps(c);
        }
		
        template<typename S>
		OverlapResult raw_overlaps(const TBVH<S>& c) const
        {
            return bvh().raw_overlaps(c.bvh());
        }

		template<typename Other>
		CollisionResult raw_collides(const Other& c) const
        {
            return bvh().raw_collides(c);
        }
		
        template<typename S>
		CollisionResult raw_collides(const BVH<S>& c) const
        {
            return bvh().raw_collides(c);
        }
		
        template<typename S>
		CollisionResult raw_collides(const TBVH<S>& c) const
        {
            return bvh().raw_collides(c.bvh());
        }

		template<typename Other>
		ContactResult raw_contacts(const Other& c) const
        {
            return bvh().raw_contacts(c);
        }
		
        template<typename S>
        ContactResult raw_contacts(const BVH<S>& c) const
        {
            return bvh().raw_contacts(c);
        }
		
		template<typename S>
		ContactResult raw_contacts(const TBVH<S>& c) const
        {
            return bvh().raw_contacts(c.bvh());
        }

    private:
        const BVH<Shape>& bvh() const
        {
            if (local_dirty_ || transformer_.flush())
            {
                local_dirty_ = false;
                const glm::mat3 m = transformer_.global();
                std::vector<Element>& global_elements = bvh_.set_elements();
                global_elements.resize(local_elements_.size());
                for (size_t i = 0; i < local_elements_.size(); ++i)
                    global_elements[i] = local_elements_[i].transformed(m);
            }

            return bvh_;
        }
	};
}
