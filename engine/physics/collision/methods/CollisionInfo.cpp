#include "CollisionInfo.h"

#include "core/math/Geometry.h"

namespace oly::col2d
{
	CollisionResult greedy_collision(const std::vector<CollisionResult>& collisions)
	{
		if (collisions.empty())
			return { .overlap = false };
		else if (collisions.size() == 1)
			return collisions[0];

		// step 1: prune redundant MTVs
		struct GreedyMTV
		{
			float signed_length;
			bool has_opposing;
		};
		std::unordered_map<UnitVector2D, GreedyMTV> mapped_mtvs;
		for (const CollisionResult& collision : collisions)
		{
			if (!collision.overlap || near_zero(collision.penetration_depth))
				continue;

			auto it = mapped_mtvs.find(collision.unit_impulse);
			if (it == mapped_mtvs.end())
			{
				auto rit = mapped_mtvs.find(-collision.unit_impulse);
				if (rit == mapped_mtvs.end())
					mapped_mtvs.insert({ collision.unit_impulse, { collision.penetration_depth, false } });
				else
				{
					if (approx(collision.penetration_depth, glm::abs(rit->second.signed_length)))
					{
						if (rit->second.signed_length > 0.0f)
							rit->second.has_opposing = true;
					}
					else if (collision.penetration_depth > glm::abs(rit->second.signed_length))
					{
						rit->second.signed_length = -collision.penetration_depth;
						rit->second.has_opposing = false;
					}
				}
			}
			else
			{
				if (approx(collision.penetration_depth, glm::abs(it->second.signed_length)))
				{
					if (it->second.signed_length < 0.0f)
						it->second.has_opposing = true;
				}
				else if (collision.penetration_depth > glm::abs(it->second.signed_length))
				{
					it->second.signed_length = collision.penetration_depth;
					it->second.has_opposing = false;
				}
			}
		}
		std::vector<glm::vec2> greediest_mtvs;
		for (const auto& [normal, mtv] : mapped_mtvs)
		{
			if (!mtv.has_opposing)
				greediest_mtvs.push_back((glm::vec2)normal * mtv.signed_length);
		}
		if (greediest_mtvs.empty())
			return { .overlap = false };

		// step 2: find greediest orthonormal frame
		glm::vec2 greediest_mtv{};
		float largest_mag_sqrd = 0.0f;
		for (glm::vec2 normal : greediest_mtvs)
		{
			UnitVector2D axis1 = normal;
			UnitVector2D axis2 = axis1.get_quarter_turn();

			float greediest_x = 0.0f;
			float greediest_y = 0.0f;
			for (glm::vec2 mtv : greediest_mtvs)
			{
				float x = axis1.dot(mtv);
				if (glm::abs(x) > glm::abs(greediest_x))
					greediest_x = x;
				float y = axis2.dot(mtv);
				if (glm::abs(y) > glm::abs(greediest_y))
					greediest_y = y;
			}
			float mag_sqrd = greediest_x * greediest_x + greediest_y * greediest_y;
			if (mag_sqrd > largest_mag_sqrd)
			{
				largest_mag_sqrd = mag_sqrd;
				greediest_mtv = greediest_x * (glm::vec2)axis1 + greediest_y * (glm::vec2)axis2;
			}
		}

		// step 3: construct CollisionResult
		return { .overlap = true, .penetration_depth = glm::length(greediest_mtv), .unit_impulse = UnitVector2D(greediest_mtv) };
	}

	ContactResult greedy_contact(const std::vector<ContactResult>& contacts)
	{
		if (contacts.empty())
			return { .overlap = false };
		else if (contacts.size() == 1)
			return contacts[0];

		// step 1: prune redundant MTVs
		struct GreedyMTV
		{
			float signed_length;
			bool has_opposing;
			glm::vec2 active_contact, static_contact;
		};
		std::unordered_map<UnitVector2D, GreedyMTV> mapped_mtvs;
		for (const ContactResult& contact : contacts)
		{
			if (!contact.overlap || near_zero(contact.active_feature.impulse))
				continue;

			float impulse_sqrd = math::mag_sqrd(contact.active_feature.impulse);
			auto it = mapped_mtvs.find(UnitVector2D(contact.active_feature.impulse));
			if (it == mapped_mtvs.end())
			{
				auto rit = mapped_mtvs.find(UnitVector2D(contact.static_feature.impulse));
				if (rit == mapped_mtvs.end())
					mapped_mtvs.insert({ UnitVector2D(contact.active_feature.impulse), { impulse_sqrd, false, contact.active_feature.position, {contact.static_feature.position}} });
				else
				{
					if (approx(impulse_sqrd, glm::abs(rit->second.signed_length)))
					{
						if (rit->second.signed_length > 0.0f)
							rit->second.has_opposing = true;
					}
					else if (impulse_sqrd > glm::abs(rit->second.signed_length))
					{
						rit->second.signed_length = -impulse_sqrd;
						rit->second.has_opposing = false;
						rit->second.active_contact = contact.active_feature.position;
						rit->second.static_contact = contact.static_feature.position;
					}
				}
			}
			else
			{
				if (approx(impulse_sqrd, glm::abs(it->second.signed_length)))
				{
					if (it->second.signed_length < 0.0f)
						it->second.has_opposing = true;
				}
				else if (impulse_sqrd > glm::abs(it->second.signed_length))
				{
					it->second.signed_length = impulse_sqrd;
					it->second.has_opposing = false;
					it->second.active_contact = contact.active_feature.position;
					it->second.static_contact = contact.static_feature.position;
				}
			}
		}

		struct GreedyContact
		{
			glm::vec2 mtv;
			glm::vec2 active_contact, static_contact;
		};
		std::vector<GreedyContact> greediest_mtvs;
		for (const auto& [normal, mtv] : mapped_mtvs)
		{
			if (!mtv.has_opposing)
				greediest_mtvs.push_back({ .mtv = (glm::vec2)normal * mtv.signed_length, .active_contact = mtv.active_contact, .static_contact = mtv.static_contact });
		}
		if (greediest_mtvs.empty())
			return { .overlap = false };

		// step 2: find greediest orthonormal frame
		GreedyContact greediest_mtv{};
		float largest_mag_sqrd = 0.0f;
		for (const GreedyContact& normal : greediest_mtvs)
		{
			UnitVector2D axis1 = normal.mtv;
			UnitVector2D axis2 = axis1.get_quarter_turn();

			glm::vec2 active_contact_x{}, active_contact_y{};
			glm::vec2 static_contact_x{}, static_contact_y{};
			float greediest_x = 0.0f;
			float greediest_y = 0.0f;
			for (const GreedyContact& mtv : greediest_mtvs)
			{
				float x = axis1.dot(mtv.mtv);
				if (glm::abs(x) > glm::abs(greediest_x))
				{
					greediest_x = x;
					active_contact_x = mtv.active_contact;
					static_contact_x = mtv.static_contact;
				}
				float y = axis2.dot(mtv.mtv);
				if (glm::abs(y) > glm::abs(greediest_y))
				{
					greediest_y = y;
					active_contact_y = mtv.active_contact;
					static_contact_y = mtv.static_contact;
				}
			}
			float mag_sqrd = greediest_x * greediest_x + greediest_y * greediest_y;
			if (mag_sqrd > largest_mag_sqrd)
			{
				largest_mag_sqrd = mag_sqrd;
				greediest_mtv.mtv = greediest_x * (glm::vec2)axis1 + greediest_y * (glm::vec2)axis2;
				float weight_x = glm::abs(greediest_x);
				float weight_y = glm::abs(greediest_y);
				float weight_total = weight_x + weight_y;
				greediest_mtv.active_contact = (weight_x * active_contact_x + weight_y * active_contact_y) / weight_total;
				greediest_mtv.static_contact = (weight_x * static_contact_x + weight_y * static_contact_y) / weight_total;
			}
		}

		// step 3: construct ContactResult
		return { .overlap = true, .active_feature = {.position = greediest_mtv.active_contact, .impulse = greediest_mtv.mtv },
			.static_feature = {.position = greediest_mtv.static_contact, .impulse = -greediest_mtv.mtv } };
	}
}
