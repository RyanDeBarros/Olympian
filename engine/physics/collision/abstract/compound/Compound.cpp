#include "Compound.h"

namespace oly::acm2d
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

	ContactResult greedy_collision(const std::vector<ContactResult>& contacts)
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
					mapped_mtvs.insert({ UnitVector2D(contact.active_feature.impulse), { impulse_sqrd, false, contact.active_feature.position, {contact.static_feature.position}}});
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
		return { .overlap = true, .active_feature = { .position = greediest_mtv.active_contact, .impulse = greediest_mtv.mtv },
			.static_feature = { .position = greediest_mtv.static_contact, .impulse = -greediest_mtv.mtv } };
	}

	OverlapResult point_hits(const Compound& c, glm::vec2 test)
	{
		for (const auto& primitive : c.primitives)
		{
			if (std::visit([test](auto&& primitive) { return point_hits(primitive, test); }, primitive))
				return true;
		}
		return false;
	}

	OverlapResult ray_hits(const Compound& c, const Ray& ray)
	{
		for (const auto& primitive : c.primitives)
		{
			if (std::visit([&ray](auto&& primitive) { return ray_hits(primitive, ray); }, primitive))
				return true;
		}
		return false;
	}

	RaycastResult raycast(const Compound& c, const Ray& ray)
	{
		RaycastResult info{ .hit = RaycastResult::Hit::NO_HIT };
		float closest_dist_sqrd = std::numeric_limits<float>::max();
		for (const auto& primitive : c.primitives)
		{
			RaycastResult res = std::visit([&ray](auto&& primitive) { return raycast(primitive, ray); }, primitive);
			if (res.hit == RaycastResult::Hit::EMBEDDED_ORIGIN)
				return res;
			else if (res.hit == RaycastResult::Hit::TRUE_HIT)
			{
				float dist_sqrd = math::mag_sqrd(res.contact - ray.origin);
				if (dist_sqrd < closest_dist_sqrd)
				{
					closest_dist_sqrd = dist_sqrd;
					info = res;
				}
			}
		}
		return info;
	}

	OverlapResult overlaps(const Compound& c1, const Compound& c2)
	{
		for (const auto& p1 : c1.primitives)
		{
			if (std::visit([&c2](auto&& p1) {
				for (const auto& p2 : c2.primitives)
				{
					if (std::visit([&p1](auto&& p2) { return overlaps(p1, p2); }, p2))
						return true;
				}
				return false;
			}, p1))
				return true;
		}
		return false;
	}

	CollisionResult collides(const Compound& c1, const Compound& c2)
	{
		std::vector<CollisionResult> collisions;
		for (const auto& p1 : c1.primitives)
		{
			std::visit([&collisions, &c2](auto&& p1) {
				for (const auto& p2 : c2.primitives)
				{
					CollisionResult collision = std::visit([&p1](auto&& p2) { return collides(p1, p2); }, p2);
					if (collision.overlap)
						collisions.push_back(collision);
				}
				}, p1);
		}
		return greedy_collision(collisions);
	}

	ContactResult contacts(const Compound& c1, const Compound& c2)
	{
		std::vector<ContactResult> cntcts;
		for (const auto& p1 : c1.primitives)
		{
			std::visit([&cntcts, &c2](auto&& p1) {
				for (const auto& p2 : c2.primitives)
				{
					ContactResult contact = std::visit([&p1](auto&& p2) { return contacts(p1, p2); }, p2);
					if (contact.overlap)
						cntcts.push_back(contact);
				}
				}, p1);
		}
		return greedy_collision(cntcts);
	}

	OverlapResult overlaps(const Compound& c1, const Primitive& c2)
	{
		for (const auto& p1 : c1.primitives)
		{
			if (std::visit([&c2](auto&& p1) { return std::visit([&p1](auto&& c2) { return overlaps(p1, c2); }, c2); }, p1))
				return true;
		}
		return false;
	}
	
	CollisionResult collides(const Compound& c1, const Primitive& c2)
	{
		std::vector<CollisionResult> collisions;
		for (const auto& p1 : c1.primitives)
		{
			std::visit([&collisions, &c2](auto&& p1) {
				CollisionResult collision = std::visit([&p1](auto&& c2) { return collides(p1, c2); }, c2);
				if (collision.overlap)
					collisions.push_back(collision);
				}, p1);
		}
		return greedy_collision(collisions);
	}
	
	ContactResult contacts(const Compound& c1, const Primitive& c2)
	{
		std::vector<ContactResult> cntcts;
		for (const auto& p1 : c1.primitives)
		{
			std::visit([&cntcts, &c2](auto&& p1) {
				ContactResult contact = std::visit([&p1](auto&& c2) { return contacts(p1, c2); }, c2);
				if (contact.overlap)
					cntcts.push_back(contact);
				}, p1);
		}
		return greedy_collision(cntcts);
	}

	OverlapResult point_hits(const TCompound& c, glm::vec2 test)
	{
		glm::vec2 local_test = glm::inverse(c.transformer.global()) * glm::vec3(test, 1.0f);
		return point_hits(c.compound, local_test);
	}
	
	OverlapResult ray_hits(const TCompound& c, const Ray& ray)
	{
		glm::mat3 m = glm::inverse(c.transformer.global());
		Ray local_ray = { .origin = m * glm::vec3(ray.origin, 1.0f) };
		if (ray.clip == 0.0f)
		{
			local_ray.direction = glm::vec2(m * glm::vec3((glm::vec2)ray.direction, 0.0f));
			local_ray.clip = 0.0f;
		}
		else
		{
			glm::vec2 clip = ray.clip * (glm::vec2)ray.direction;
			clip = m * glm::vec3(clip, 0.0f);
			local_ray.direction = clip;
			local_ray.clip = glm::length(clip);
		}
		return ray_hits(c.compound, local_ray);
	}
	
	RaycastResult raycast(const TCompound& c, const Ray& ray)
	{
		glm::mat3 g = c.transformer.global();
		glm::mat3 m = glm::inverse(g);
		Ray local_ray = { .origin = m * glm::vec3(ray.origin, 1.0f) };
		if (ray.clip == 0.0f)
		{
			local_ray.direction = glm::vec2(m * glm::vec3((glm::vec2)ray.direction, 0.0f));
			local_ray.clip = 0.0f;
		}
		else
		{
			glm::vec2 clip = ray.clip * (glm::vec2)ray.direction;
			clip = m * glm::vec3(clip, 0.0f);
			local_ray.direction = clip;
			local_ray.clip = glm::length(clip);
		}
		RaycastResult result = raycast(c.compound, local_ray);
		result.contact = transform_point(g, result.contact);
		result.normal = transform_normal(g, result.normal);
		return result;
	}
	
	OverlapResult overlaps(const TCompound& c1, const TCompound& c2)
	{
		glm::mat3 g1 = c1.transformer.global();
		glm::mat3 g2 = c2.transformer.global();
		for (const auto& p1 : c1.compound.primitives)
		{
			if (std::visit([&g1, &g2, &c2](auto&& p1) {
				for (const auto& p2 : c2.compound.primitives)
				{

					if (std::visit([&g1, &g2, &p1](auto&& p2) {
						bool transform_1 = true;
						if constexpr (std::is_same_v<std::decay_t<decltype(p1)>, Circle>)
						{
							transform_1 = false;
						}
						else if constexpr (!std::is_same_v<std::decay_t<decltype(p2)>, Circle>)
						{
							if (degree(p1) > degree(p2))
								transform_1 = false;
						}

						if (transform_1)
						{
							// TODO
							return overlaps(, p2);
						}
						else
						{
							// TODO
							return overlaps(p1, );
						}
						}, p2))
						return true;
				}
				return false;
				}, p1))
				return true;
		}
		return false;
	}
	
	CollisionResult collides(const TCompound& c1, const TCompound& c2)
	{

	}
	
	ContactResult contacts(const TCompound& c1, const TCompound& c2)
	{

	}
}
