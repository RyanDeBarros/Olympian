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
		std::vector<CollisionResult> collisions;
		for (const auto& e1 : c1.get_baked())
		{
			CollisionResult collision = collides(param(e1), c2);
			if (collision.overlap)
				collisions.push_back(collision);
		}
		return greedy_collision(collisions, c1.get_baked().data(), c1.get_baked().size(), c2);
	}

	ContactResult contacts(const TCompound& c1, ElementParam c2)
	{
		std::vector<ContactResult> cntcts;
		for (const auto& e1 : c1.get_baked())
		{
			ContactResult contact = contacts(param(e1), c2);
			if (contact.overlap)
				cntcts.push_back(contact);
		}
		return greedy_contact(cntcts, c1.get_baked().data(), c1.get_baked().size(), c2);
	}
}
