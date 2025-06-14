#include "Combinations.h"

namespace oly::col2d
{
	OverlapResult internal::overlaps(const TCompound& c1, const TCompound& c2)
	{
		for (const auto& e1 : c1.get_baked())
			for (const auto& e2 : c2.get_baked())
				if (overlaps(e1, e2))
					return true;
		return false;
	}

	CollisionResult internal::collides(const TCompound& c1, const TCompound& c2)
	{
		std::vector<CollisionResult> collisions;
		for (const auto& e1 : c1.get_baked())
		{
			for (const auto& e2 : c2.get_baked())
			{
				CollisionResult collision = collides(e1, e2);
				if (collision.overlap)
					collisions.push_back(collision);
			}
		}
		return greedy_collision(collisions);
	}

	ContactResult internal::contacts(const TCompound& c1, const TCompound& c2)
	{
		std::vector<ContactResult> cntcts;
		for (const auto& e1 : c1.get_baked())
		{
			for (const auto& e2 : c2.get_baked())
			{
				ContactResult contact = contacts(e1, e2);
				if (contact.overlap)
					cntcts.push_back(contact);
			}
		}
		return greedy_contact(cntcts);
	}

	OverlapResult internal::overlaps(const TCompound& c1, const Compound& c2)
	{
		for (const auto& e1 : c1.get_baked())
			for (const auto& e2 : c2.elements)
				if (overlaps(e1, e2))
					return true;
		return false;
	}

	CollisionResult internal::collides(const TCompound& c1, const Compound& c2)
	{
		std::vector<CollisionResult> collisions;
		for (const auto& e1 : c1.get_baked())
		{
			for (const auto& e2 : c2.elements)
			{
				CollisionResult collision = collides(e1, e2);
				if (collision.overlap)
					collisions.push_back(collision);
			}
		}
		return greedy_collision(collisions);
	}

	ContactResult internal::contacts(const TCompound& c1, const Compound& c2)
	{
		std::vector<ContactResult> cntcts;
		for (const auto& e1 : c1.get_baked())
		{
			for (const auto& e2 : c2.elements)
			{
				ContactResult contact = contacts(e1, e2);
				if (contact.overlap)
					cntcts.push_back(contact);
			}
		}
		return greedy_contact(cntcts);
	}

	OverlapResult overlaps(const TCompound& c1, const Element& c2)
	{
		for (const auto& e1 : c1.get_baked())
			if (overlaps(e1, c2))
				return true;
		return false;
	}

	CollisionResult collides(const TCompound& c1, const Element& c2)
	{
		std::vector<CollisionResult> collisions;
		for (const auto& e1 : c1.get_baked())
		{
			CollisionResult collision = collides(e1, c2);
			if (collision.overlap)
				collisions.push_back(collision);
		}
		return greedy_collision(collisions);
	}

	ContactResult contacts(const TCompound& c1, const Element& c2)
	{
		std::vector<ContactResult> cntcts;
		for (const auto& e1 : c1.get_baked())
		{
			ContactResult contact = contacts(e1, c2);
			if (contact.overlap)
				cntcts.push_back(contact);
		}
		return greedy_contact(cntcts);
	}
}
