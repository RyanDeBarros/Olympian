#include "Element.h"

#include "core/base/Transforms.h"
#include "core/base/SimpleMath.h"

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

	static bool only_translation_and_scale(const glm::mat3& m)
	{
		return (near_zero(m[0][1]) && near_zero(m[1][0])) || (near_zero(m[0][0]) && near_zero(m[1][1]));
	}

	static bool orthogonal_transform(const glm::mat3& m)
	{
		return near_zero(glm::dot(m[0], m[1]));
	}

	Element transform_element(const Circle& c, const glm::mat3& m)
	{
		Circle tc(c.center, c.radius);
		internal::CircleGlobalAccess::set_global(tc, m);
		return tc;
	}

	Element transform_element(const AABB& c, const glm::mat3& m)
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
			return OBB{ .center = m * glm::vec3(c.center(), 1.0f), .width = c.width() * glm::length(m[0]), .height = c.height() * glm::length(m[1]), .rotation = glm::atan(m[0][1], m[0][0])};
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

			return CustomKDOP::wrap_copy_ptr(polygon, axes);
		}
	}

	Element transform_element(const OBB& c, const glm::mat3& m)
	{
		if (orthogonal_transform(m))
		{
			float rotation = glm::atan(m[0][1], m[0][0]);
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
				return OBB{ .center = transform_point(m, c.center), .width = glm::length(m[0]) * c.width, .height = glm::length(m[1]) * c.height, .rotation = c.rotation + rotation };
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

			return CustomKDOP::wrap_copy_ptr(polygon, axes);
		}
	}

	Element transform_element(const CopyPtr<CustomKDOP>& c, const glm::mat3& m)
	{
		// CustomKDOP
		// LATER check if compatible with standard KDOP axes
		std::vector<UnitVector2D> axes(c->get_k());
		std::vector<float> minima(c->get_k());
		std::vector<float> maxima(c->get_k());
		for (size_t i = 0; i < axes.size(); ++i)
		{
			axes[i] = transform_normal(m, c->edge_normal(i));
			float offset = axes[i].dot(m[2]);
			minima[i] = offset + axes[i].dot(transform_normal(m, c->get_minimum(i) * (glm::vec2)c->edge_normal(i)));
			maxima[i] = offset + axes[i].dot(transform_normal(m, c->get_maximum(i) * (glm::vec2)c->edge_normal(i)));
		}
		return make_copy_ptr<CustomKDOP>(std::move(axes), std::move(minima), std::move(maxima));
	}

	template<size_t K>
	static Element transform_element_impl(const CopyPtr<KDOP<K>>& c, const glm::mat3& m)
	{
		bool reverse_axes = false;
		bool maintain_axes = false;
		float scale = 0.0f;
		float rotation = 0.0f;
		if (orthogonal_transform(m))
		{
			scale = glm::length(m[0]);
			float scaleY = glm::length(m[1]);
			reverse_axes = math::cross(m[0], m[1]) < 0.0f;
			rotation = glm::atan(m[0][1], m[0][0]);
			if (approx(glm::abs(scale), glm::abs(scaleY)) && near_multiple(rotation, glm::pi<float>() / K))
				maintain_axes = true;
		}

		if (maintain_axes)
		{
			// KDOP
			glm::vec2 translation = m[2];
			int rotation_axis_offset = 0;
			rotation_axis_offset = roundi(K * rotation * glm::one_over_pi<float>());
			std::array<float, K> minima;
			std::array<float, K> maxima;
			for (int i = 0; i < K; ++i)
			{
				float offset = KDOP<K>::uniform_axis(i).dot(translation);
				int og_idx = reverse_axes ? int(K) - i : i;
				og_idx = unsigned_mod(og_idx + rotation_axis_offset, int(K));
				minima[i] = c->get_minimum(og_idx) * scale + offset;
				maxima[i] = c->get_maximum(og_idx) * scale + offset;
			}
			return make_copy_ptr<KDOP<K>>(minima, maxima);
		}
		else
		{
			// CustomKDOP
			std::vector<UnitVector2D> axes(K);
			std::vector<float> minima(K);
			std::vector<float> maxima(K);
			for (size_t i = 0; i < axes.size(); ++i)
			{
				axes[i] = transform_normal(m, KDOP<K>::uniform_axis(i));
				float offset = axes[i].dot(m[2]);
				minima[i] = offset + axes[i].dot(transform_normal(m, c->get_minimum(i) * (glm::vec2)KDOP<K>::uniform_axis(i)));
				maxima[i] = offset + axes[i].dot(transform_normal(m, c->get_maximum(i) * (glm::vec2)KDOP<K>::uniform_axis(i)));
			}
			return make_copy_ptr<CustomKDOP>(std::move(axes), std::move(minima), std::move(maxima));
		}
	}

	Element transform_element(const CopyPtr<KDOP3>& c, const glm::mat3& m)
	{
		return transform_element_impl(c, m);
	}

	Element transform_element(const CopyPtr<KDOP4>& c, const glm::mat3& m)
	{
		return transform_element_impl(c, m);
	}

	Element transform_element(const CopyPtr<KDOP5>& c, const glm::mat3& m)
	{
		return transform_element_impl(c, m);
	}

	Element transform_element(const CopyPtr<KDOP6>& c, const glm::mat3& m)
	{
		return transform_element_impl(c, m);
	}

	Element transform_element(const CopyPtr<KDOP7>& c, const glm::mat3& m)
	{
		return transform_element_impl(c, m);
	}

	Element transform_element(const CopyPtr<KDOP8>& c, const glm::mat3& m)
	{
		return transform_element_impl(c, m);
	}

	Element transform_element(const ConvexHull& c, const glm::mat3& m)
	{
		ConvexHull tc;
		std::vector<glm::vec2>& points = tc.set_points();
		points.reserve(points.size());
		for (glm::vec2 p : points)
			points.push_back(transform_point(m, p));
		return tc;
	}

}
