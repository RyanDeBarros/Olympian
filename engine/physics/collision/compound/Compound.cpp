#include "Compound.h"

#include "core/base/SimpleMath.h"

// TODO in all collision testing, use approximations and override tolerance to be more lenient.
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

	static bool only_translation_and_scale(const glm::mat3& m)
	{
		return (near_zero(m[0][1]) && near_zero(m[1][0])) || (near_zero(m[0][0]) && near_zero(m[1][1]));
	}

	static bool orthogonal_transform(const glm::mat3& m)
	{
		return near_zero(glm::dot(m[0], m[1]));
	}

	Primitive transform_primitive(const Circle& c, const glm::mat3& m)
	{
		Circle tc(c.center, c.radius);
		internal::CircleGlobalAccess::set_global(tc, m);
		return tc;
	}

	Primitive transform_primitive(const AABB& c, const glm::mat3& m)
	{
		if (only_translation_and_scale(m))
		{
			// AABB
			if (near_zero(m[0][1]))
			{
				float x1 = m[0][0] * c.x1 + m[2][0];
				float x2 = m[0][0] * c.x2 + m[2][0];
				float y1 = m[1][1] * c.y1 + m[2][1];
				float y2 = m[1][1] * c.y2 + m[2][1];
				return AABB{ .x1 = std::min(x1, x2), .x2 = std::max(x1, x2), .y1 = std::min(y1, y2), .y2 = std::max(y1, y2) };
			}
			else
			{
				float x1 = m[1][0] * c.y1 + m[2][0];
				float x2 = m[1][0] * c.y2 + m[2][0];
				float y1 = m[0][1] * c.x1 + m[2][1];
				float y2 = m[0][1] * c.x2 + m[2][1];
				return AABB{ .x1 = std::min(x1, x2), .x2 = std::max(x1, x2), .y1 = std::min(y1, y2), .y2 = std::max(y1, y2) };
			}
		}
		else if (orthogonal_transform(m))
		{
			// OBB
			glm::vec2 scale = extract_scale(m);
			return OBB{ .center = m * glm::vec3(c.center(), 1.0f), .width = (c.x2 - c.x1) * scale.x, .height = (c.y2 - c.y1) * scale.y, .rotation = extract_rotation(m)};
		}
		else
		{
			// CustomKDOP
			std::vector<UnitVector2D> axes = {
				transform_direction(m, UnitVector2D::RIGHT),
				transform_direction(m, UnitVector2D::UP)
				};
			
			math::Polygon2D polygon;
			polygon.reserve(4);
			for (glm::vec2 point : c.points())
				polygon.push_back(transform_point(m, point));

			return CustomKDOP::wrap(polygon, axes);
		}
	}

	Primitive transform_primitive(const OBB& c, const glm::mat3& m)
	{
		if (orthogonal_transform(m))
		{
			float rotation = extract_rotation(m);
			float r = fmod((c.rotation + rotation) * glm::two_over_pi<float>(), 1.0f);
			if (near_zero(r) || approx(r, 1.0f))
			{
				// AABB
				math::Polygon2D polygon;
				polygon.reserve(4);
				for (glm::vec2 point : c.points())
					polygon.push_back(transform_point(m, point));

				return AABB::wrap(polygon.data(), polygon.size());
			}
			else
			{
				// OBB
				glm::vec2 scale = extract_scale(m);
				return OBB{ .center = transform_point(m, c.center), .width = scale.x * c.width, .height = scale.y * c.height, .rotation = c.rotation + rotation};
			}
		}
		else
		{
			// CustomKDOP
			std::vector<UnitVector2D> axes = {
				transform_direction(m, c.get_major_axis()),
				transform_direction(m, c.get_minor_axis())
			};

			math::Polygon2D polygon;
			polygon.reserve(4);
			for (glm::vec2 point : c.points())
				polygon.push_back(transform_point(m, point));

			return CustomKDOP::wrap(polygon, axes);
		}
	}

	Primitive transform_primitive(const CustomKDOP& c, const glm::mat3& m)
	{
		math::Polygon2D polygon;
		polygon.reserve(c.points().size());
		for (glm::vec2 point : c.points())
			polygon.push_back(transform_point(m, point));

		std::vector<UnitVector2D> axes(c.get_axes().size());
		for (size_t i = 0; i < axes.size(); ++i)
			axes[i] = transform_direction(m, c.get_axes()[i]);

		if (c.get_k_half() == 2)
		{
			if (near_zero(axes[0].dot(axes[1])))
			{
				if (axes[0].near_standard() && axes[1].near_standard())
				{
					// AABB
					return AABB::wrap(polygon.data(), polygon.size());
				}
				else
				{
					// OBB
					return OBB::fast_wrap(polygon.data(), polygon.size());
				}
			}
			else
			{
				// CustomKDOP
				return CustomKDOP::wrap(polygon, axes);
			}
		}
		else
		{
			// CustomKDOP
			// LATER check if compatible with standard KDOP axes
			return CustomKDOP::wrap(polygon, axes);
		}
	}

	// TODO test this transformation especially
	template<size_t K_half>
	static Primitive transform_primitive_impl(const KDOP<K_half>& c, const glm::mat3& m)
	{
		bool maintain_axes = false;
		glm::vec2 scale2D{};
		float rotation = 0.0f;
		if (orthogonal_transform(m))
		{
			scale2D = extract_scale(m);
			rotation = extract_rotation(m);
			if (approx(glm::abs(scale2D.x), glm::abs(scale2D.y)) && near_multiple(rotation, glm::pi<float>() / K_half))
				maintain_axes = true;
		}

		if (maintain_axes)
		{
			// KDOP
			glm::vec2 translation = m[2];
			int rotation_axis_offset = 0;
			rotation_axis_offset = roundi(K_half * rotation * glm::one_over_pi<float>());
			// TODO In LaTeX documentation, use graphic to explain this more visually, but keep comment here. Or, use syntax "SEE chapter#.section#.subsubection# in doc/Olympian.pdf". SEE allows me to easily find and update these comments.
			// Use scale2D.y as the representative scale, i.e. the sign of scale2D.y determines the sign of the scale that will multiply the extrema. This effect is illustrated here:
			// 
			// 1. scale2D.x > 0 && scale2D.y > 0
			//     -> No reflection, axes preserved, and scale remains positive.
			// 
			// 2. scale2D.x < 0 && scale2D.y > 0
			//     -> Reflection across Y-axis, so reverse_axes = true. Note that reversing the axis order is equivalent to a horizontal reflection, so scale remains positive.
			// 
			// 3. scale2D.x > 0 && scale2D.y < 0
			//     -> Reflection across X-axis. Given an axis in quadrant I, its reflection into quadrant IV is equivalent to a reflection into quadrant II but with negative scale.
			//        Therefore, reverse_axes = true, but scale becomes negative.
			// 
			// 4. scale2D.x < 0 && scale2D.y < 0
			//     -> 180-degree rotation. This is equivalent to *not* reversing axis order, but simply negating the scale.
			// 
			// Therefore, reverse_axes = sign(scale2D.x) != sign(scale2D.y) and scale = scale2D.y.
			bool reverse_axes = glm::sign(scale2D.x) != glm::sign(scale2D.y);
			float scale = scale2D.y;

			std::array<float, K_half> minima;
			std::array<float, K_half> maxima;
			for (int i = 0; i < K_half; ++i)
			{
				float offset = KDOP<K_half>::uniform_axis(i).dot(translation);
				int og_idx = reverse_axes ? int(K_half) - i : i;
				og_idx = unsigned_mod(og_idx + rotation_axis_offset, int(K_half));
				minima[i] = c.get_minimum(og_idx) * scale + offset;
				maxima[i] = c.get_maximum(og_idx) * scale + offset;
			}
			return KDOP<K_half>(minima, maxima);
		}
		else
		{
			// CustomKDOP
			math::Polygon2D polygon;
			polygon.reserve(c.points().size());
			for (glm::vec2 point : c.points())
				polygon.push_back(transform_point(m, point));

			std::vector<UnitVector2D> axes(K_half);
			for (size_t i = 0; i < axes.size(); ++i)
				axes[i] = transform_direction(m, KDOP<K_half>::uniform_axis(i));

			return CustomKDOP::wrap(polygon, axes);
		}
	}

	Primitive transform_primitive(const KDOP6& c, const glm::mat3& m)
	{
		return transform_primitive_impl(c, m);
	}

	Primitive transform_primitive(const KDOP8& c, const glm::mat3& m)
	{
		return transform_primitive_impl(c, m);
	}

	Primitive transform_primitive(const KDOP10& c, const glm::mat3& m)
	{
		return transform_primitive_impl(c, m);
	}

	Primitive transform_primitive(const KDOP12& c, const glm::mat3& m)
	{
		return transform_primitive_impl(c, m);
	}

	Primitive transform_primitive(const KDOP14& c, const glm::mat3& m)
	{
		return transform_primitive_impl(c, m);
	}

	Primitive transform_primitive(const KDOP16& c, const glm::mat3& m)
	{
		return transform_primitive_impl(c, m);
	}

	Primitive transform_primitive(const ConvexHull& c, const glm::mat3& m)
	{
		ConvexHull tc;
		std::vector<glm::vec2>& points = tc.set_points();
		points.reserve(points.size());
		for (glm::vec2 p : points)
			points.push_back(transform_point(m, p));
		return tc;
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

	OverlapResult internal::overlaps(const Compound& c1, const Compound& c2)
	{
		// TODO cache AABB of compound as a whole, and then compare here for early-out
		for (const auto& p1 : c1.primitives)
		{
			if (std::visit([&c2](auto&& p1) {
				for (const auto& p2 : c2.primitives)
				{
					// TODO BVH-like check for AABB/OBB of p1/p2 first, if primitive is kDOP or ConvexHull of degree >= 6. Put in SAT/GJK instead of here?
					if (std::visit([&p1](auto&& p2) { return overlaps(p1, p2); }, p2))
						return true;
				}
				return false;
			}, p1))
				return true;
		}
		return false;
	}

	CollisionResult internal::collides(const Compound& c1, const Compound& c2)
	{
		// TODO cache AABB of compound as a whole, and then compare here for early-out
		std::vector<CollisionResult> collisions;
		for (const auto& p1 : c1.primitives)
		{
			std::visit([&collisions, &c2](auto&& p1) {
				for (const auto& p2 : c2.primitives)
				{
					// TODO BVH-like check for AABB/OBB of p1/p2 first, if primitive is kDOP or ConvexHull of degree >= 6. Put in SAT/GJK instead of here?
					CollisionResult collision = std::visit([&p1](auto&& p2) { return collides(p1, p2); }, p2);
					if (collision.overlap)
						collisions.push_back(collision);
				}
				}, p1);
		}
		return greedy_collision(collisions);
	}

	ContactResult internal::contacts(const Compound& c1, const Compound& c2)
	{
		// TODO cache AABB of compound as a whole, and then compare here for early-out
		std::vector<ContactResult> cntcts;
		for (const auto& p1 : c1.primitives)
		{
			std::visit([&cntcts, &c2](auto&& p1) {
				for (const auto& p2 : c2.primitives)
				{
					// TODO BVH-like check for AABB/OBB of p1/p2 first, if primitive is kDOP or ConvexHull of degree >= 6. Put in SAT/GJK instead of here?
					ContactResult contact = std::visit([&p1](auto&& p2) { return contacts(p1, p2); }, p2);
					if (contact.overlap)
						cntcts.push_back(contact);
				}
				}, p1);
		}
		return greedy_contact(cntcts);
	}

	OverlapResult overlaps(const Compound& c1, const Primitive& c2)
	{
		// TODO cache AABB of compound as a whole, and then compare here for early-out
		for (const auto& p1 : c1.primitives)
		{
			// TODO BVH-like check for AABB/OBB of p1/p2 first, if primitive is kDOP or ConvexHull of degree >= 6. Put in SAT/GJK instead of here?
			if (std::visit([&c2](auto&& p1) { return std::visit([&p1](auto&& c2) { return overlaps(p1, c2); }, c2); }, p1))
				return true;
		}
		return false;
	}
	
	CollisionResult collides(const Compound& c1, const Primitive& c2)
	{
		// TODO cache AABB of compound as a whole, and then compare here for early-out
		std::vector<CollisionResult> collisions;
		for (const auto& p1 : c1.primitives)
		{
			// TODO BVH-like check for AABB/OBB of p1/p2 first, if primitive is kDOP or ConvexHull of degree >= 6. Put in SAT/GJK instead of here?
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
		// TODO cache AABB of compound as a whole, and then compare here for early-out
		std::vector<ContactResult> cntcts;
		for (const auto& p1 : c1.primitives)
		{
			// TODO BVH-like check for AABB/OBB of p1/p2 first, if primitive is kDOP or ConvexHull of degree >= 6. Put in SAT/GJK instead of here?
			std::visit([&cntcts, &c2](auto&& p1) {
				ContactResult contact = std::visit([&p1](auto&& c2) { return contacts(p1, c2); }, c2);
				if (contact.overlap)
					cntcts.push_back(contact);
				}, p1);
		}
		return greedy_contact(cntcts);
	}

	void TCompound::bake() const
	{
		baked.resize(compound.primitives.size());
		glm::mat3 g = transformer.global();
		for (size_t i = 0; i < baked.size(); ++i)
			baked[i] = std::visit([&g](auto&& p) { return transform_primitive(p, g); }, compound.primitives[i]);
		dirty = false;
	}

	OverlapResult point_hits(const TCompound& c, glm::vec2 test)
	{
		glm::vec2 local_test = transform_point(glm::inverse(c.global()), test);
		return point_hits(c.get_compound(), local_test);
	}
	
	OverlapResult ray_hits(const TCompound& c, const Ray& ray)
	{
		glm::mat3 m = glm::inverse(c.global());
		Ray local_ray = { .origin = transform_point(m, ray.origin) };
		if (ray.clip == 0.0f)
		{
			local_ray.direction = transform_direction(m, ray.direction);
			local_ray.clip = 0.0f;
		}
		else
		{
			glm::vec2 clip = transform_direction(m, ray.clip * (glm::vec2)ray.direction);
			local_ray.direction = clip;
			local_ray.clip = glm::length(clip);
		}
		return ray_hits(c.get_compound(), local_ray);
	}
	
	RaycastResult raycast(const TCompound& c, const Ray& ray)
	{
		glm::mat3 g = c.global();
		glm::mat3 m = glm::inverse(g);
		Ray local_ray = { .origin = transform_point(m, ray.origin) };
		if (ray.clip == 0.0f)
		{
			local_ray.direction = transform_direction(m, ray.direction);
			local_ray.clip = 0.0f;
		}
		else
		{
			glm::vec2 clip = transform_direction(m, ray.clip * (glm::vec2)ray.direction);
			local_ray.direction = clip;
			local_ray.clip = glm::length(clip);
		}
		RaycastResult result = raycast(c.get_compound(), local_ray);
		result.contact = transform_point(g, result.contact);
		result.normal = transform_normal(g, result.normal);
		return result;
	}
	
	OverlapResult internal::overlaps(const TCompound& c1, const TCompound& c2)
	{
		// TODO cache AABB of compound as a whole, and then compare here for early-out
		for (const auto& p1 : c1.get_baked())
		{
			if (std::visit([&c2](auto&& p1) {
				for (const auto& p2 : c2.get_baked())
				{
					// TODO BVH-like check for AABB/OBB of p1/p2 first, if primitive is kDOP or ConvexHull of degree >= 6. Put in SAT/GJK instead of here?
					if (std::visit([&p1](auto&& p2) { return overlaps(p1, p2); }, p2))
						return true;
				}
				return false;
				}, p1))
				return true;
		}
		return false;
	}
	
	CollisionResult internal::collides(const TCompound& c1, const TCompound& c2)
	{
		// TODO cache AABB of compound as a whole, and then compare here for early-out
		std::vector<CollisionResult> collisions;
		for (const auto& p1 : c1.get_baked())
		{
			std::visit([&collisions, &c2](auto&& p1) {
				for (const auto& p2 : c2.get_baked())
				{
					// TODO BVH-like check for AABB/OBB of p1/p2 first, if primitive is kDOP or ConvexHull of degree >= 6. Put in SAT/GJK instead of here?
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
		// TODO cache AABB of compound as a whole, and then compare here for early-out
		std::vector<ContactResult> cntcts;
		for (const auto& p1 : c1.get_baked())
		{
			std::visit([&cntcts, &c2](auto&& p1) {
				for (const auto& p2 : c2.get_baked())
				{
					// TODO BVH-like check for AABB/OBB of p1/p2 first, if primitive is kDOP or ConvexHull of degree >= 6. Put in SAT/GJK instead of here?
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
		// TODO cache AABB of compound as a whole, and then compare here for early-out
		for (const auto& p1 : c1.get_baked())
		{
			if (std::visit([&c2](auto&& p1) {
				for (const auto& p2 : c2.primitives)
				{
					// TODO BVH-like check for AABB/OBB of p1/p2 first, if primitive is kDOP or ConvexHull of degree >= 6. Put in SAT/GJK instead of here?
					if (std::visit([&p1](auto&& p2) { return overlaps(p1, p2); }, p2))
						return true;
				}
				return false;
				}, p1))
				return true;
		}
		return false;
	}

	CollisionResult internal::collides(const TCompound& c1, const Compound& c2)
	{
		// TODO cache AABB of compound as a whole, and then compare here for early-out
		std::vector<CollisionResult> collisions;
		for (const auto& p1 : c1.get_baked())
		{
			std::visit([&collisions, &c2](auto&& p1) {
				for (const auto& p2 : c2.primitives)
				{
					// TODO BVH-like check for AABB/OBB of p1/p2 first, if primitive is kDOP or ConvexHull of degree >= 6. Put in SAT/GJK instead of here?
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
		// TODO cache AABB of compound as a whole, and then compare here for early-out
		std::vector<ContactResult> cntcts;
		for (const auto& p1 : c1.get_baked())
		{
			std::visit([&cntcts, &c2](auto&& p1) {
				for (const auto& p2 : c2.primitives)
				{
					// TODO BVH-like check for AABB/OBB of p1/p2 first, if primitive is kDOP or ConvexHull of degree >= 6. Put in SAT/GJK instead of here?
					ContactResult contact = std::visit([&p1](auto&& p2) { return contacts(p1, p2); }, p2);
					if (contact.overlap)
						cntcts.push_back(contact);
				}
				}, p1);
		}
		return greedy_contact(cntcts);
	}

	OverlapResult overlaps(const TCompound& c1, const Primitive& c2)
	{
		// TODO cache AABB of compound as a whole, and then compare here for early-out
		for (const auto& p1 : c1.get_baked())
		{
			// TODO BVH-like check for AABB/OBB of p1/p2 first, if primitive is kDOP or ConvexHull of degree >= 6. Put in SAT/GJK instead of here?
			if (std::visit([&c2](auto&& p1) { return std::visit([&p1](auto&& c2) { return overlaps(p1, c2); }, c2); }, p1))
				return true;
		}
		return false;
	}

	CollisionResult collides(const TCompound& c1, const Primitive& c2)
	{
		// TODO cache AABB of compound as a whole, and then compare here for early-out
		std::vector<CollisionResult> collisions;
		for (const auto& p1 : c1.get_baked())
		{
			// TODO BVH-like check for AABB/OBB of p1/p2 first, if primitive is kDOP or ConvexHull of degree >= 6. Put in SAT/GJK instead of here?
			std::visit([&collisions, &c2](auto&& p1) {
				CollisionResult collision = std::visit([&p1](auto&& c2) { return collides(p1, c2); }, c2);
				if (collision.overlap)
					collisions.push_back(collision);
				}, p1);
		}
		return greedy_collision(collisions);
	}

	ContactResult contacts(const TCompound& c1, const Primitive& c2)
	{
		// TODO cache AABB of compound as a whole, and then compare here for early-out
		std::vector<ContactResult> cntcts;
		for (const auto& p1 : c1.get_baked())
		{
			// TODO BVH-like check for AABB/OBB of p1/p2 first, if primitive is kDOP or ConvexHull of degree >= 6. Put in SAT/GJK instead of here?
			std::visit([&cntcts, &c2](auto&& p1) {
				ContactResult contact = std::visit([&p1](auto&& c2) { return contacts(p1, c2); }, c2);
				if (contact.overlap)
					cntcts.push_back(contact);
				}, p1);
		}
		return greedy_contact(cntcts);
	}
}
