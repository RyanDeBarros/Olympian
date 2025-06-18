#include "Combinations.h"

namespace oly::col2d
{
	OverlapResult overlaps(const TCompound& c1, ElementParam c2)
	{
		for (const auto& e1 : c1.get_baked())
			if (overlaps(param(e1), c2))
				return true;
		return false;
	}

	CollisionResult collides(const TCompound& c1, ElementParam c2)
	{
		return compound_collision(c1.get_baked().data(), c1.get_baked().size(), c2);
	}

	ContactResult contacts(const TCompound& c1, ElementParam c2)
	{
		return compound_contact(c1.get_baked().data(), c1.get_baked().size(), c2);
	}
}
