#pragma once

#include "core/base/TransformerExposure.h"
#include "core/base/Transforms.h"

#include "physics/collision/Tolerance.h"

#include "physics/collision/elements/Element.h"

#include "physics/collision/methods/Collide.h"
#include "physics/collision/methods/KDOPCollide.h"

namespace oly::col2d
{
	struct Primitive
	{
		Element element;

		Mask mask = 0;
		Layer layer = 0;

		float projection_max(UnitVector2D axis) const;
		float projection_min(UnitVector2D axis) const;
		fpair projection_interval(UnitVector2D axis) const;
		ContactManifold deepest_manifold(UnitVector2D axis) const;

		template<internal::Elem_check Shape>
		const Shape& element_as() const
		{
			return *element.variant().get<const Shape*>();
		}

		template<internal::Elem_check Shape>
		Shape& element_as()
		{
			return *element.variant().get<Shape*>();
		}
	};

    extern OverlapResult point_hits(const Primitive& c, glm::vec2 test);
    extern OverlapResult ray_hits(const Primitive& c, Ray ray);
    extern RaycastResult raycast(const Primitive& c, Ray ray);

	extern OverlapResult overlaps(const Primitive& c1, const Primitive& c2);
	extern CollisionResult collides(const Primitive& c1, const Primitive& c2);
	extern ContactResult contacts(const Primitive& c1, const Primitive& c2);

	namespace internal { struct LUT; };

	class TPrimitive
	{
		friend struct internal::LUT;

		Transformer2D transformer_;
		Primitive primitive_;
		mutable Element baked_;
		mutable bool dirty_ = true;

	public:
		TPrimitive() = default;
        explicit TPrimitive(Element element) noexcept;

        const Transform2D& get_local() const;
        Transform2D& set_local();
        bool is_dirty() const;

        Transformer2DExposure<> get_transformer() const;
        Transformer2DExposure<ex::kFull> set_transformer();

        const Primitive& get_primitive() const;
        Primitive& set_primitive();
        const Element& get_baked() const;

        Mask mask() const;
        Mask& mask();
        Layer layer() const;
        Layer& layer();

		float projection_max(UnitVector2D axis) const;
		float projection_min(UnitVector2D axis) const;
		fpair projection_interval(UnitVector2D axis) const;
        ContactManifold deepest_manifold(UnitVector2D axis) const;

		template<internal::Elem_check Shape>
		const Shape& element_as() const
		{
			return get_primitive().element_as<Shape>();
		}

		template<internal::Elem_check Shape>
		Shape& element_as()
		{
			return set_primitive().element_as<Shape>();
		}

    private:
        void bake() const;
	};

    extern OverlapResult point_hits(const TPrimitive& c, glm::vec2 test);
    extern OverlapResult ray_hits(const TPrimitive& c, Ray ray);
    extern RaycastResult raycast(const TPrimitive& c, Ray ray);

	extern OverlapResult overlaps(const TPrimitive& c1, const TPrimitive& c2);
	extern OverlapResult overlaps(const TPrimitive& c1, const Primitive& c2);
	extern OverlapResult overlaps(const Primitive& c1, const TPrimitive& c2);
	extern CollisionResult collides(const TPrimitive& c1, const TPrimitive& c2);
	extern CollisionResult collides(const TPrimitive& c1, const Primitive& c2);
	extern CollisionResult collides(const Primitive& c1, const TPrimitive& c2);
	extern ContactResult contacts(const TPrimitive& c1, const TPrimitive& c2);
	extern ContactResult contacts(const TPrimitive& c1, const Primitive& c2);
	extern ContactResult contacts(const Primitive& c1, const TPrimitive& c2);

	extern OverlapResult overlaps(const Primitive& c1, const Element& c2);
	extern OverlapResult overlaps(const Element& c1, const Primitive& c2);
	extern CollisionResult collides(const Primitive& c1, const Element& c2);
	extern CollisionResult collides(const Element& c1, const Primitive& c2);
	extern ContactResult contacts(const Primitive& c1, const Element& c2);
	extern ContactResult contacts(const Element& c1, const Primitive& c2);

	extern OverlapResult overlaps(const TPrimitive& c1, const Element& c2);
	extern OverlapResult overlaps(const Element& c1, const TPrimitive& c2);
	extern CollisionResult collides(const TPrimitive& c1, const Element& c2);
	extern CollisionResult collides(const Element& c1, const TPrimitive& c2);
	extern ContactResult contacts(const TPrimitive& c1, const Element& c2);
	extern ContactResult contacts(const Element& c1, const TPrimitive& c2);
}
