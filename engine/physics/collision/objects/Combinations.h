#pragma once

#include "physics/collision/objects/Primitive.h"
#include "physics/collision/objects/Compound.h"
#include "physics/collision/objects/BVH.h"
#include "physics/collision/Tolerance.h"

namespace oly::col2d
{
	template<typename Shape>
	inline BVH<Shape> convert(const Compound& compound)
	{
		BVH<Shape> bvh;
		bvh.set_elements() = compound.elements;
		bvh.layer = compound.layer;
		bvh.mask = compound.mask;
		return bvh;
	}
	
	template<typename Shape>
	inline BVH<Shape> convert(Compound&& compound)
	{
		BVH<Shape> bvh;
		bvh.set_elements() = std::move(compound.elements);
		bvh.layer = compound.layer;
		bvh.mask = compound.mask;
		return bvh;
	}

	template<typename Shape>
	inline TBVH<Shape> convert(const TCompound& compound)
	{
		TBVH<Shape> bvh;
		bvh.transformer = compound.transformer;
		bvh.set_elements() = compound.get_compound().elements;
		bvh.layer() = compound.layer();
		bvh.mask() = compound.layer();
		return bvh;
	}

	template<typename Shape>
	inline TBVH<Shape> convert(TCompound&& compound)
	{
		TBVH<Shape> bvh;
		bvh.transformer = std::move(compound.transformer);
		bvh.set_elements() = std::move(compound.set_compound().elements);
		bvh.layer() = compound.layer();
		bvh.mask() = compound.layer();
		return bvh;
	}

	inline OverlapResult overlaps(const Compound& c1, const Primitive& c2) { return (c1.mask & c2.layer) && overlaps(c1, c2.element); }
	inline OverlapResult overlaps(const Primitive& c1, const Compound& c2) { return overlaps(c2, c1); }
	inline CollisionResult collides(const Compound& c1, const Primitive& c2) { return (c1.mask & c2.layer) ? collides(c1, c2.element) : CollisionResult{ .overlap = false }; }
	inline CollisionResult collides(const Primitive& c1, const Compound& c2) { return collides(c2, c1).invert(); }
	inline ContactResult contacts(const Compound& c1, const Primitive& c2) { return (c1.mask & c2.layer) ? contacts(c1, c2.element) : ContactResult{ .overlap = false }; }
	inline ContactResult contacts(const Primitive& c1, const Compound& c2) { return contacts(c2, c1).invert(); }

	inline OverlapResult overlaps(const TCompound& c1, const Primitive& c2) { return (c1.mask() & c2.layer) && overlaps(c1, c2.element); }
	inline OverlapResult overlaps(const Primitive& c1, const TCompound& c2) { return overlaps(c2, c1); }
	inline CollisionResult collides(const TCompound& c1, const Primitive& c2) { return (c1.mask() & c2.layer) ? collides(c1, c2.element) : CollisionResult{ .overlap = false }; }
	inline CollisionResult collides(const Primitive& c1, const TCompound& c2) { return collides(c2, c1).invert(); }
	inline ContactResult contacts(const TCompound& c1, const Primitive& c2) { return (c1.mask() & c2.layer) ? contacts(c1, c2.element) : ContactResult{ .overlap = false }; }
	inline ContactResult contacts(const Primitive& c1, const TCompound& c2) { return contacts(c2, c1).invert(); }

	inline OverlapResult overlaps(const Compound& c1, const TPrimitive& c2) { return (c1.mask & c2.layer()) && overlaps(c1, c2.get_baked()); }
	inline OverlapResult overlaps(const TPrimitive& c1, const Compound& c2) { return overlaps(c2, c1); }
	inline CollisionResult collides(const Compound& c1, const TPrimitive& c2) { return (c1.mask & c2.layer()) ? collides(c1, c2.get_baked()) : CollisionResult{ .overlap = false }; }
	inline CollisionResult collides(const TPrimitive& c1, const Compound& c2) { return collides(c2, c1).invert(); }
	inline ContactResult contacts(const Compound& c1, const TPrimitive& c2) { return (c1.mask & c2.layer()) ? contacts(c1, c2.get_baked()) : ContactResult{ .overlap = false }; }
	inline ContactResult contacts(const TPrimitive& c1, const Compound& c2) { return contacts(c2, c1).invert(); }

	inline OverlapResult overlaps(const TCompound& c1, const TPrimitive& c2) { return (c1.mask() & c2.layer()) && overlaps(c1, c2.get_baked()); }
	inline OverlapResult overlaps(const TPrimitive& c1, const TCompound& c2) { return overlaps(c2, c1); }
	inline CollisionResult collides(const TCompound& c1, const TPrimitive& c2) { return (c1.mask() & c2.layer()) ? collides(c1, c2.get_baked()) : CollisionResult{ .overlap = false }; }
	inline CollisionResult collides(const TPrimitive& c1, const TCompound& c2) { return collides(c2, c1).invert(); }
	inline ContactResult contacts(const TCompound& c1, const TPrimitive& c2) { return (c1.mask() & c2.layer()) ? contacts(c1, c2.get_baked()) : ContactResult{ .overlap = false }; }
	inline ContactResult contacts(const TPrimitive& c1, const TCompound& c2) { return contacts(c2, c1).invert(); }

	template<typename Shape>
	inline OverlapResult point_hits(const BVH<Shape>& c, glm::vec2 test) { return c.point_hits(test); }
	template<typename Shape>
	inline OverlapResult ray_hits(const BVH<Shape>& c, Ray ray) { return c.ray_hits(ray); }
	template<typename Shape>
	inline RaycastResult raycast(const BVH<Shape>& c, Ray ray) { return c.raycast(ray); }
	template<typename Shape>
	inline OverlapResult point_hits(const TBVH<Shape>& c, glm::vec2 test) { return c.point_hits(test); }
	template<typename Shape>
	inline OverlapResult ray_hits(const TBVH<Shape>& c, Ray ray) { return c.ray_hits(ray); }
	template<typename Shape>
	inline RaycastResult raycast(const TBVH<Shape>& c, Ray ray) { return c.raycast(ray); }

	template<typename Shape>
	inline OverlapResult overlaps(const BVH<Shape>& c1, const Element& c2) { return c1.raw_overlaps(c2); }
	template<typename Shape>
	inline OverlapResult overlaps(const Element& c1, const BVH<Shape>& c2) { return c2.raw_overlaps(c1); }
	template<typename Shape>
	inline CollisionResult collides(const BVH<Shape>& c1, const Element& c2) { return c1.raw_collides(c2); }
	template<typename Shape>
	inline CollisionResult collides(const Element& c1, const BVH<Shape>& c2) { return c2.raw_collides(c1).invert(); }
	template<typename Shape>
	inline ContactResult contacts(const BVH<Shape>& c1, const Element& c2) { return c1.raw_contacts(c2); }
	template<typename Shape>
	inline ContactResult contacts(const Element& c1, const BVH<Shape>& c2) { return c2.raw_contacts(c1).invert(); }

	template<typename Shape>
	inline OverlapResult overlaps(const TBVH<Shape>& c1, const Element& c2) { return c1.raw_overlaps(c2); }
	template<typename Shape>
	inline OverlapResult overlaps(const Element& c1, const TBVH<Shape>& c2) { return c2.raw_overlaps(c1); }
	template<typename Shape>
	inline CollisionResult collides(const TBVH<Shape>& c1, const Element& c2) { return c1.raw_collides(c2); }
	template<typename Shape>
	inline CollisionResult collides(const Element& c1, const TBVH<Shape>& c2) { return c2.raw_collides(c1).invert(); }
	template<typename Shape>
	inline ContactResult contacts(const TBVH<Shape>& c1, const Element& c2) { return c1.raw_contacts(c2); }
	template<typename Shape>
	inline ContactResult contacts(const Element& c1, const TBVH<Shape>& c2) { return c2.raw_contacts(c1).invert(); }

	template<typename Shape>
	inline OverlapResult overlaps(const BVH<Shape>& c1, const Primitive& c2) { return (c1.mask & c2.layer) && c1.raw_overlaps(c2); }
	template<typename Shape>
	inline OverlapResult overlaps(const Primitive& c1, const BVH<Shape>& c2) { return (c1.mask & c2.layer) && c2.raw_overlaps(c1); }
	template<typename Shape>
	inline CollisionResult collides(const BVH<Shape>& c1, const Primitive& c2) { return (c1.mask & c2.layer) ? c1.raw_collides(c2) : CollisionResult{ .overlap = false }; }
	template<typename Shape>
	inline CollisionResult collides(const Primitive& c1, const BVH<Shape>& c2) { return (c1.mask & c2.layer) ? c2.raw_collides(c1).invert() : CollisionResult{ .overlap = false }; }
	template<typename Shape>
	inline ContactResult contacts(const BVH<Shape>& c1, const Primitive& c2) { return (c1.mask & c2.layer) ? c1.raw_contacts(c2) : ContactResult{ .overlap = false }; }
	template<typename Shape>
	inline ContactResult contacts(const Primitive& c1, const BVH<Shape>& c2) { return (c1.mask & c2.layer) ? c2.raw_contacts(c1).invert() : ContactResult{ .overlap = false }; }

	template<typename Shape>
	inline OverlapResult overlaps(const TBVH<Shape>& c1, const Primitive& c2) { return (c1.mask() & c2.layer) && c1.raw_overlaps(c2); }
	template<typename Shape>
	inline OverlapResult overlaps(const Primitive& c1, const TBVH<Shape>& c2) { return (c1.mask & c2.layer()) && c2.raw_overlaps(c1); }
	template<typename Shape>
	inline CollisionResult collides(const TBVH<Shape>& c1, const Primitive& c2) { return (c1.mask() & c2.layer) ? c1.raw_collides(c2) : CollisionResult{ .overlap = false }; }
	template<typename Shape>
	inline CollisionResult collides(const Primitive& c1, const TBVH<Shape>& c2) { return (c1.mask & c2.layer()) ? c2.raw_collides(c1).invert() : CollisionResult{ .overlap = false }; }
	template<typename Shape>
	inline ContactResult contacts(const TBVH<Shape>& c1, const Primitive& c2) { return (c1.mask() & c2.layer) ? c1.raw_contacts(c2) : ContactResult{ .overlap = false }; }
	template<typename Shape>
	inline ContactResult contacts(const Primitive& c1, const TBVH<Shape>& c2) { return (c1.mask & c2.layer()) ? c2.raw_contacts(c1).invert() : ContactResult{ .overlap = false }; }

	template<typename Shape>
	inline OverlapResult overlaps(const BVH<Shape>& c1, const TPrimitive& c2) { return (c1.mask & c2.layer()) && c1.raw_overlaps(c2); }
	template<typename Shape>
	inline OverlapResult overlaps(const TPrimitive& c1, const BVH<Shape>& c2) { return (c1.mask() & c2.layer) && c2.raw_overlaps(c1); }
	template<typename Shape>
	inline CollisionResult collides(const BVH<Shape>& c1, const TPrimitive& c2) { return (c1.mask & c2.layer()) ? c1.raw_collides(c2) : CollisionResult{ .overlap = false }; }
	template<typename Shape>
	inline CollisionResult collides(const TPrimitive& c1, const BVH<Shape>& c2) { return (c1.mask() & c2.layer) ? c2.raw_collides(c1).invert() : CollisionResult{ .overlap = false }; }
	template<typename Shape>
	inline ContactResult contacts(const BVH<Shape>& c1, const TPrimitive& c2) { return (c1.mask & c2.layer()) ? c1.raw_contacts(c2) : ContactResult{ .overlap = false }; }
	template<typename Shape>
	inline ContactResult contacts(const TPrimitive& c1, const BVH<Shape>& c2) { return (c1.mask() & c2.layer) ? c2.raw_contacts(c1).invert() : ContactResult{ .overlap = false }; }

	template<typename Shape>
	inline OverlapResult overlaps(const TBVH<Shape>& c1, const TPrimitive& c2) { return (c1.mask() & c2.layer()) && c1.raw_overlaps(c2); }
	template<typename Shape>
	inline OverlapResult overlaps(const TPrimitive& c1, const TBVH<Shape>& c2) { return (c1.mask() & c2.layer()) && c2.raw_overlaps(c1); }
	template<typename Shape>
	inline CollisionResult collides(const TBVH<Shape>& c1, const TPrimitive& c2) { return (c1.mask() & c2.layer()) ? c1.raw_collides(c2) : CollisionResult{ .overlap = false }; }
	template<typename Shape>
	inline CollisionResult collides(const TPrimitive& c1, const TBVH<Shape>& c2) { return (c1.mask() & c2.layer()) ? c2.raw_collides(c1).invert() : CollisionResult{ .overlap = false }; }
	template<typename Shape>
	inline ContactResult contacts(const TBVH<Shape>& c1, const TPrimitive& c2) { return (c1.mask() & c2.layer()) ? c1.raw_contacts(c2) : ContactResult{ .overlap = false }; }
	template<typename Shape>
	inline ContactResult contacts(const TPrimitive& c1, const TBVH<Shape>& c2) { return (c1.mask() & c2.layer()) ? c2.raw_contacts(c1).invert() : ContactResult{ .overlap = false }; }

	template<typename Shape>
	inline OverlapResult overlaps(const BVH<Shape>& c1, const Compound& c2) { return (c1.mask & c2.layer) && c1.raw_overlaps(c2); }
	template<typename Shape>
	inline OverlapResult overlaps(const Compound& c1, const BVH<Shape>& c2) { return (c1.mask & c2.layer) && c2.raw_overlaps(c1); }
	template<typename Shape>
	inline CollisionResult collides(const BVH<Shape>& c1, const Compound& c2) { return (c1.mask & c2.layer) ? c1.raw_collides(c2) : CollisionResult{ .overlap = false }; }
	template<typename Shape>
	inline CollisionResult collides(const Compound& c1, const BVH<Shape>& c2) { return (c1.mask & c2.layer) ? c2.raw_collides(c1).invert() : CollisionResult{ .overlap = false }; }
	template<typename Shape>
	inline ContactResult contacts(const BVH<Shape>& c1, const Compound& c2) { return (c1.mask & c2.layer) ? c1.raw_contacts(c2) : ContactResult{ .overlap = false }; }
	template<typename Shape>
	inline ContactResult contacts(const Compound& c1, const BVH<Shape>& c2) { return (c1.mask & c2.layer) ? c2.raw_contacts(c1).invert() : ContactResult{ .overlap = false }; }

	template<typename Shape>
	inline OverlapResult overlaps(const TBVH<Shape>& c1, const Compound& c2) { return (c1.mask() & c2.layer) && c1.raw_overlaps(c2); }
	template<typename Shape>
	inline OverlapResult overlaps(const Compound& c1, const TBVH<Shape>& c2) { return (c1.mask & c2.layer()) && c2.raw_overlaps(c1); }
	template<typename Shape>
	inline CollisionResult collides(const TBVH<Shape>& c1, const Compound& c2) { return (c1.mask() & c2.layer) ? c1.raw_collides(c2) : CollisionResult{ .overlap = false }; }
	template<typename Shape>
	inline CollisionResult collides(const Compound& c1, const TBVH<Shape>& c2) { return (c1.mask & c2.layer()) ? c2.raw_collides(c1).invert() : CollisionResult{ .overlap = false }; }
	template<typename Shape>
	inline ContactResult contacts(const TBVH<Shape>& c1, const Compound& c2) { return (c1.mask() & c2.layer) ? c1.raw_contacts(c2) : ContactResult{ .overlap = false }; }
	template<typename Shape>
	inline ContactResult contacts(const Compound& c1, const TBVH<Shape>& c2) { return (c1.mask & c2.layer()) ? c2.raw_contacts(c1).invert() : ContactResult{ .overlap = false }; }

	template<typename Shape>
	inline OverlapResult overlaps(const BVH<Shape>& c1, const TCompound& c2) { return (c1.mask & c2.layer()) && c1.raw_overlaps(c2); }
	template<typename Shape>
	inline OverlapResult overlaps(const TCompound& c1, const BVH<Shape>& c2) { return (c1.mask() & c2.layer) && c2.raw_overlaps(c1); }
	template<typename Shape>
	inline CollisionResult collides(const BVH<Shape>& c1, const TCompound& c2) { return (c1.mask & c2.layer()) ? c1.raw_collides(c2) : CollisionResult{ .overlap = false }; }
	template<typename Shape>
	inline CollisionResult collides(const TCompound& c1, const BVH<Shape>& c2) { return (c1.mask() & c2.layer) ? c2.raw_collides(c1).invert() : CollisionResult{ .overlap = false }; }
	template<typename Shape>
	inline ContactResult contacts(const BVH<Shape>& c1, const TCompound& c2) { return (c1.mask & c2.layer()) ? c1.raw_contacts(c2) : ContactResult{ .overlap = false }; }
	template<typename Shape>
	inline ContactResult contacts(const TCompound& c1, const BVH<Shape>& c2) { return (c1.mask() & c2.layer) ? c2.raw_contacts(c1).invert() : ContactResult{ .overlap = false }; }

	template<typename Shape>
	inline OverlapResult overlaps(const TBVH<Shape>& c1, const TCompound& c2) { return (c1.mask() & c2.layer()) && c1.raw_overlaps(c2); }
	template<typename Shape>
	inline OverlapResult overlaps(const TCompound& c1, const TBVH<Shape>& c2) { return (c1.mask() & c2.layer()) && c2.raw_overlaps(c1); }
	template<typename Shape>
	inline CollisionResult collides(const TBVH<Shape>& c1, const TCompound& c2) { return (c1.mask() & c2.layer()) ? c1.raw_collides(c2) : CollisionResult{ .overlap = false }; }
	template<typename Shape>
	inline CollisionResult collides(const TCompound& c1, const TBVH<Shape>& c2) { return (c1.mask() & c2.layer()) ? c2.raw_collides(c1).invert() : CollisionResult{ .overlap = false }; }
	template<typename Shape>
	inline ContactResult contacts(const TBVH<Shape>& c1, const TCompound& c2) { return (c1.mask() & c2.layer()) ? c1.raw_contacts(c2) : ContactResult{ .overlap = false }; }
	template<typename Shape>
	inline ContactResult contacts(const TCompound& c1, const TBVH<Shape>& c2) { return (c1.mask() & c2.layer()) ? c2.raw_contacts(c1).invert() : ContactResult{ .overlap = false }; }

	template<typename Shape1, typename Shape2>
	inline OverlapResult overlaps(const BVH<Shape1>& c1, const BVH<Shape2>& c2) { return (c1.mask & c2.layer) && c1.raw_overlaps(c2); }
	template<typename Shape1, typename Shape2>
	inline CollisionResult collides(const BVH<Shape1>& c1, const BVH<Shape2>& c2) { return (c1.mask & c2.layer) ? c1.raw_collides(c2) : CollisionResult{ .overlap = false }; }
	template<typename Shape1, typename Shape2>
	inline ContactResult contacts(const BVH<Shape1>& c1, const BVH<Shape2>& c2) { return (c1.mask & c2.layer) ? c1.raw_contacts(c2) : ContactResult{ .overlap = false }; }
	template<typename Shape1, typename Shape2>
	inline OverlapResult overlaps(const TBVH<Shape1>& c1, const TBVH<Shape2>& c2) { return (c1.mask() & c2.layer()) && c1.raw_overlaps(c2); }
	template<typename Shape1, typename Shape2>
	inline CollisionResult collides(const TBVH<Shape1>& c1, const TBVH<Shape2>& c2) { return (c1.mask() & c2.layer()) ? c1.raw_collides(c2) : CollisionResult{ .overlap = false }; }
	template<typename Shape1, typename Shape2>
	inline ContactResult contacts(const TBVH<Shape1>& c1, const TBVH<Shape2>& c2) { return (c1.mask() & c2.layer()) ? c1.raw_contacts(c2) : ContactResult{ .overlap = false }; }

	template<typename Shape1, typename Shape2>
	inline OverlapResult overlaps(const TBVH<Shape1>& c1, const BVH<Shape2>& c2) { return (c1.mask() & c2.layer) && c1.raw_overlaps(c2); }
	template<typename Shape1, typename Shape2>
	inline OverlapResult overlaps(const BVH<Shape1>& c1, const TBVH<Shape2>& c2) { return (c1.mask & c2.layer()) && c2.raw_overlaps(c1); }
	template<typename Shape1, typename Shape2>
	inline CollisionResult collides(const TBVH<Shape1>& c1, const BVH<Shape2>& c2) { return (c1.mask() & c2.layer) ? c1.raw_collides(c2) : CollisionResult{ .overlap = false }; }
	template<typename Shape1, typename Shape2>
	inline CollisionResult collides(const BVH<Shape1>& c1, const TBVH<Shape2>& c2) { return (c1.mask & c2.layer()) ? c2.raw_collides(c1).invert() : CollisionResult{ .overlap = false }; }
	template<typename Shape1, typename Shape2>
	inline ContactResult contacts(const TBVH<Shape1>& c1, const BVH<Shape2>& c2) { return (c1.mask() & c2.layer) ? c1.raw_contacts(c2) : ContactResult{ .overlap = false }; }
	template<typename Shape1, typename Shape2>
	inline ContactResult contacts(const BVH<Shape1>& c1, const TBVH<Shape2>& c2) { return (c1.mask & c2.layer()) ? c2.raw_contacts(c1).invert() : ContactResult{ .overlap = false }; }
}
