#include "Combinations.h"

namespace oly::col2d
{
    OverlapResult overlaps(const Compound& c1, const Primitive& c2)
    {
        return (c1.mask & c2.layer) && overlaps(c1, c2.element);
    }

    OverlapResult overlaps(const Primitive& c1, const Compound& c2)
    {
        return overlaps(c2, c1);
    }

    CollisionResult collides(const Compound& c1, const Primitive& c2)
    {
        return (c1.mask & c2.layer) ? collides(c1, c2.element) : CollisionResult{ .overlap = false };
    }

    CollisionResult collides(const Primitive& c1, const Compound& c2)
    {
        return collides(c2, c1).invert();
    }

    ContactResult contacts(const Compound& c1, const Primitive& c2)
    {
        return (c1.mask & c2.layer) ? contacts(c1, c2.element) : ContactResult{ .overlap = false };
    }

    ContactResult contacts(const Primitive& c1, const Compound& c2)
    {
        return contacts(c2, c1).invert();
    }

    OverlapResult overlaps(const TCompound& c1, const Primitive& c2)
    {
        return (c1.mask() & c2.layer) && overlaps(c1, c2.element);
    }

    OverlapResult overlaps(const Primitive& c1, const TCompound& c2)
    {
        return overlaps(c2, c1);
    }

    CollisionResult collides(const TCompound& c1, const Primitive& c2)
    {
        return (c1.mask() & c2.layer) ? collides(c1, c2.element) : CollisionResult{ .overlap = false };
    }

    CollisionResult collides(const Primitive& c1, const TCompound& c2)
    {
        return collides(c2, c1).invert();
    }

    ContactResult contacts(const TCompound& c1, const Primitive& c2)
    {
        return (c1.mask() & c2.layer) ? contacts(c1, c2.element) : ContactResult{ .overlap = false };
    }

    ContactResult contacts(const Primitive& c1, const TCompound& c2)
    {
        return contacts(c2, c1).invert();
    }

    OverlapResult overlaps(const Compound& c1, const TPrimitive& c2)
    {
        return (c1.mask & c2.layer()) && overlaps(c1, c2.get_baked());
    }

    OverlapResult overlaps(const TPrimitive& c1, const Compound& c2)
    {
        return overlaps(c2, c1);
    }

    CollisionResult collides(const Compound& c1, const TPrimitive& c2)
    {
        return (c1.mask & c2.layer()) ? collides(c1, c2.get_baked()) : CollisionResult{ .overlap = false };
    }

    CollisionResult collides(const TPrimitive& c1, const Compound& c2)
    {
        return collides(c2, c1).invert();
    }

    ContactResult contacts(const Compound& c1, const TPrimitive& c2)
    {
        return (c1.mask & c2.layer()) ? contacts(c1, c2.get_baked()) : ContactResult{ .overlap = false };
    }

    ContactResult contacts(const TPrimitive& c1, const Compound& c2)
    {
        return contacts(c2, c1).invert();
    }

    OverlapResult overlaps(const TCompound& c1, const TPrimitive& c2)
    {
        return (c1.mask() & c2.layer()) && overlaps(c1, c2.get_baked());
    }

    OverlapResult overlaps(const TPrimitive& c1, const TCompound& c2)
    {
        return overlaps(c2, c1);
    }

    CollisionResult collides(const TCompound& c1, const TPrimitive& c2)
    {
        return (c1.mask() & c2.layer()) ? collides(c1, c2.get_baked()) : CollisionResult{ .overlap = false };
    }

    CollisionResult collides(const TPrimitive& c1, const TCompound& c2)
    {
        return collides(c2, c1).invert();
    }

    ContactResult contacts(const TCompound& c1, const TPrimitive& c2)
    {
        return (c1.mask() & c2.layer()) ? contacts(c1, c2.get_baked()) : ContactResult{ .overlap = false };
    }

    ContactResult contacts(const TPrimitive& c1, const TCompound& c2)
    {
        return contacts(c2, c1).invert();
    }
}
