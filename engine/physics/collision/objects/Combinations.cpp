#include "Combinations.h"

namespace oly::col2d
{
	OverlapResult internal::overlaps(const TCompound& c1, const TCompound& c2)
	{
		for (const auto& p1 : c1.get_baked())
		{
			if (std::visit([&c2](auto&& p1) {
				for (const auto& p2 : c2.get_baked())
					if (std::visit([&p1](auto&& p2) { return overlaps(p1, p2); }, p2))
						return true;
				return false;
				}, p1))
				return true;
		}
		return false;
	}

	CollisionResult internal::collides(const TCompound& c1, const TCompound& c2)
	{
		std::vector<CollisionResult> collisions;
		for (const auto& p1 : c1.get_baked())
		{
			std::visit([&collisions, &c2](auto&& p1) {
				for (const auto& p2 : c2.get_baked())
				{
					CollisionResult collision = std::visit([&p1](auto&& p2) { return collides(p1, p2); }, p2);
					if (collision.overlap)
						collisions.push_back(collision);
				}
				}, p1);
		}
		return greedy_collision(collisions);
	}

	ContactResult internal::contacts(const TCompound& c1, const TCompound& c2)
	{
		std::vector<ContactResult> cntcts;
		for (const auto& p1 : c1.get_baked())
		{
			std::visit([&cntcts, &c2](auto&& p1) {
				for (const auto& p2 : c2.get_baked())
				{
					ContactResult contact = std::visit([&p1](auto&& p2) { return contacts(p1, p2); }, p2);
					if (contact.overlap)
						cntcts.push_back(contact);
				}
				}, p1);
		}
		return greedy_contact(cntcts);
	}

	OverlapResult internal::overlaps(const TCompound& c1, const Compound& c2)
	{
		for (const auto& p1 : c1.get_baked())
		{
			if (std::visit([&c2](auto&& p1) {
				for (const auto& p2 : c2.elements)
					if (std::visit([&p1](auto&& p2) { return overlaps(p1, p2); }, p2))
						return true;
				return false;
				}, p1))
				return true;
		}
		return false;
	}

	CollisionResult internal::collides(const TCompound& c1, const Compound& c2)
	{
		std::vector<CollisionResult> collisions;
		for (const auto& p1 : c1.get_baked())
		{
			std::visit([&collisions, &c2](auto&& p1) {
				for (const auto& p2 : c2.elements)
				{
					CollisionResult collision = std::visit([&p1](auto&& p2) { return collides(p1, p2); }, p2);
					if (collision.overlap)
						collisions.push_back(collision);
				}
				}, p1);
		}
		return greedy_collision(collisions);
	}

	ContactResult internal::contacts(const TCompound& c1, const Compound& c2)
	{
		std::vector<ContactResult> cntcts;
		for (const auto& p1 : c1.get_baked())
		{
			std::visit([&cntcts, &c2](auto&& p1) {
				for (const auto& p2 : c2.elements)
				{
					ContactResult contact = std::visit([&p1](auto&& p2) { return contacts(p1, p2); }, p2);
					if (contact.overlap)
						cntcts.push_back(contact);
				}
				}, p1);
		}
		return greedy_contact(cntcts);
	}

	OverlapResult overlaps(const TCompound& c1, const Element& c2)
	{
		for (const auto& p1 : c1.get_baked())
			if (std::visit([&c2](auto&& p1) { return std::visit([&p1](auto&& c2) { return overlaps(p1, c2); }, c2); }, p1))
				return true;
		return false;
	}

	CollisionResult collides(const TCompound& c1, const Element& c2)
	{
		std::vector<CollisionResult> collisions;
		for (const auto& p1 : c1.get_baked())
		{
			std::visit([&collisions, &c2](auto&& p1) {
				CollisionResult collision = std::visit([&p1](auto&& c2) { return collides(p1, c2); }, c2);
				if (collision.overlap)
					collisions.push_back(collision);
				}, p1);
		}
		return greedy_collision(collisions);
	}

	ContactResult contacts(const TCompound& c1, const Element& c2)
	{
		std::vector<ContactResult> cntcts;
		for (const auto& p1 : c1.get_baked())
		{
			std::visit([&cntcts, &c2](auto&& p1) {
				ContactResult contact = std::visit([&p1](auto&& c2) { return contacts(p1, c2); }, c2);
				if (contact.overlap)
					cntcts.push_back(contact);
				}, p1);
		}
		return greedy_contact(cntcts);
	}
}
