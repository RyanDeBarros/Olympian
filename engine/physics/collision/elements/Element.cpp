#include "Element.h"

#include "core/base/Transforms.h"
#include "physics/collision/methods/Collide.h"
#include "physics/collision/methods/KDOPCollide.h"

namespace oly::col2d
{
#define OLY_ELEMENT_IMPL_SWITCH_CASE(Macro, Class)\
	case internal::ElementIDTrait<Class>::ID:\
		Macro(obj.cast<Class>())\
		break;

#define OLY_ELEMENT_IMPL_FULL_SWITCH(Macro)\
	switch (id)\
	{\
		OLY_ELEMENT_IMPL_SWITCH_CASE(Macro, Circle);\
		OLY_ELEMENT_IMPL_SWITCH_CASE(Macro, AABB);\
		OLY_ELEMENT_IMPL_SWITCH_CASE(Macro, OBB);\
		OLY_ELEMENT_IMPL_SWITCH_CASE(Macro, ConvexHull);\
		OLY_ELEMENT_IMPL_SWITCH_CASE(Macro, KDOP2);\
		OLY_ELEMENT_IMPL_SWITCH_CASE(Macro, KDOP3);\
		OLY_ELEMENT_IMPL_SWITCH_CASE(Macro, KDOP4);\
		OLY_ELEMENT_IMPL_SWITCH_CASE(Macro, KDOP5);\
		OLY_ELEMENT_IMPL_SWITCH_CASE(Macro, KDOP6);\
		OLY_ELEMENT_IMPL_SWITCH_CASE(Macro, KDOP7);\
		OLY_ELEMENT_IMPL_SWITCH_CASE(Macro, KDOP8);\
		default:\
			throw Error(ErrorCode::UNSUPPORTED_SWITCH_CASE);\
	}

	float Element::projection_max(UnitVector2D axis) const
	{
#define OLY_ELEMENT_PROJECTION_MAX(p) return p->projection_max(axis);
		OLY_ELEMENT_IMPL_FULL_SWITCH(OLY_ELEMENT_PROJECTION_MAX);
#undef OLY_ELEMENT_PROJECTION_MAX
	}

	float Element::projection_min(UnitVector2D axis) const
	{
#define OLY_ELEMENT_PROJECTION_MIN(p) return p->projection_min(axis);
		OLY_ELEMENT_IMPL_FULL_SWITCH(OLY_ELEMENT_PROJECTION_MIN);
#undef OLY_ELEMENT_PROJECTION_MIN
	}

	fpair Element::projection_interval(UnitVector2D axis) const
	{
#define OLY_ELEMENT_PROJECTION_INTERVAL(p) return p->projection_interval(axis);
		OLY_ELEMENT_IMPL_FULL_SWITCH(OLY_ELEMENT_PROJECTION_INTERVAL);
#undef OLY_ELEMENT_PROJECTION_INTERVAL
	}

	ContactManifold Element::deepest_manifold(UnitVector2D axis) const
	{
#define OLY_ELEMENT_PROJECTION_DEEPEST_MANIFOLD(p) return p->deepest_manifold(axis);
		OLY_ELEMENT_IMPL_FULL_SWITCH(OLY_ELEMENT_PROJECTION_DEEPEST_MANIFOLD);
#undef OLY_ELEMENT_PROJECTION_DEEPEST_MANIFOLD
	}

	static bool only_translation_and_scale(const glm::mat3& m)
	{
		return (near_zero(m[0][1]) && near_zero(m[1][0])) || (near_zero(m[0][0]) && near_zero(m[1][1]));
	}

	static bool orthogonal_transform(const glm::mat3& m)
	{
		return near_zero(glm::dot(m[0], m[1]));
	}

	static Element transform_element(const Circle& c, const glm::mat3& m)
	{
		return internal::CircleGlobalAccess::create_affine_circle(c, m);
	}

	static Element transform_element(const AABB& c, const glm::mat3& m)
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
			return OBB{ .center = m * glm::vec3(c.center(), 1.0f), .width = c.width() * glm::length(m[0]), .height = c.height() * glm::length(m[1]), .rotation = glm::atan(m[0][1], m[0][0]) };
		}
		else
		{
			// KDOP
			KDOP<2> kdop({ { c.x1, c.x2 }, { c.y1, c.y2 } });
			return internal::KDOPGlobalAccess<2>::create_affine_kdop(kdop, m);
		}
	}

	static Element transform_element(const OBB& c, const glm::mat3& m)
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
			// KDOP
			KDOP<2> kdop({ { -0.5f * c.width, 0.5f * c.width}, { -0.5f * c.height, 0.5f * c.height } });
			glm::mat3 g = m * Transform2D{ .position = c.center, .rotation = c.rotation }.matrix();
			return internal::KDOPGlobalAccess<2>::create_affine_kdop(kdop, g);
		}
	}

	template<size_t K, typename = std::enable_if_t<(K > 2)>>
	static Element transform_element(const KDOP<K>& c, const glm::mat3& m)
	{
		// KDOP
		return internal::KDOPGlobalAccess<K>::create_affine_kdop(c, m);
	}

	static Element transform_element(const KDOP2& c, const glm::mat3& m)
	{
		glm::mat3 global = m * augment(internal::KDOPGlobalAccess<2>::get_global(c), internal::KDOPGlobalAccess<2>::get_global_offset(c));
		glm::mat2 g_inv_t = glm::transpose(glm::inverse(glm::mat2(global)));
		UnitVector2D right = UnitVector2D(g_inv_t * KDOP2::uniform_axis(0));
		UnitVector2D up = UnitVector2D(g_inv_t * KDOP2::uniform_axis(1));
		if (near_zero(right.dot(up)))
		{
			math::Polygon2D points = c.points();
			for (size_t i = 0; i < 4; ++i)
				points[i] = transform_point(m, points[i]);

			if (right.near_cardinal(LINEAR_TOLERANCE))
			{
				// AABB
				return AABB{
					.x1 = min(points[0].x, points[1].x, points[2].x, points[3].x),
					.x2 = max(points[0].x, points[1].x, points[2].x, points[3].x),
					.y1 = min(points[1].y, points[1].y, points[2].y, points[3].y),
					.y2 = max(points[1].y, points[1].y, points[2].y, points[3].y)
				};
			}
			else
			{
				// OBB
				UnitVector2D rot = glm::vec2(global[0]);
				glm::mat2 inverse_rotation = rot.inverse_rotation_matrix();
				for (size_t i = 0; i < 4; ++i)
					points[i] = inverse_rotation * points[i];

				AABB aabb{
					.x1 = min(points[0].x, points[1].x, points[2].x, points[3].x),
					.x2 = max(points[0].x, points[1].x, points[2].x, points[3].x),
					.y1 = min(points[1].y, points[1].y, points[2].y, points[3].y),
					.y2 = max(points[1].y, points[1].y, points[2].y, points[3].y)
				};

				return OBB{ .center = rot.rotation_matrix() * aabb.center(), .width = aabb.width(), .height = aabb.height(), .rotation = rot.rotation() };
			}
		}
		else
			return internal::KDOPGlobalAccess<2>::create_affine_kdop(c, m);
	}

	static Element transform_element(const ConvexHull& c, const glm::mat3& m)
	{
		ConvexHull tc;
		const std::vector<glm::vec2>& points = c.points();
		std::vector<glm::vec2>& tpoints = tc.set_points();
		tpoints.reserve(points.size());
		for (glm::vec2 p : points)
			tpoints.push_back(transform_point(m, p));
		return tc;
	}

	Element Element::transformed(const glm::mat3& m) const
	{
#define OLY_ELEMENT_TRANSFORMED(p) return transform_element(*p, m);
		OLY_ELEMENT_IMPL_FULL_SWITCH(OLY_ELEMENT_TRANSFORMED);
#undef OLY_ELEMENT_TRANSFORMED
	}

	Element::ElementVariant Element::variant() const
	{
#define OLY_ELEMENT_VARIANT(p) return p;
		OLY_ELEMENT_IMPL_FULL_SWITCH(OLY_ELEMENT_VARIANT);
#undef OLY_ELEMENT_VARIANT
	}

	AABB Element::aabb_wrap() const
	{
		if (id == internal::ElementID::AABB)
			return *obj.cast<AABB>();

#define OLY_ELEMENT_AABB_WRAP(p)\
		{\
			fpair ix = p->projection_interval(UnitVector2D::RIGHT);\
			fpair iy = p->projection_interval(UnitVector2D::UP);\
			return AABB{ .x1 = ix.first, .x2 = ix.second, .y1 = iy.first, .y2 = iy.second };\
		}

		OLY_ELEMENT_IMPL_FULL_SWITCH(OLY_ELEMENT_AABB_WRAP);
#undef OLY_ELEMENT_AABB_WRAP
	}

	OverlapResult Element::point_hits(glm::vec2 test) const
	{
#define OLY_ELEMENT_POINT_HITS(p) return col2d::point_hits(*p, test);
		OLY_ELEMENT_IMPL_FULL_SWITCH(OLY_ELEMENT_POINT_HITS);
#undef OLY_ELEMENT_POINT_HITS
	}

	OverlapResult Element::ray_hits(Ray ray) const
	{
#define OLY_ELEMENT_RAY_HITS(p) return col2d::ray_hits(*p, ray);
		OLY_ELEMENT_IMPL_FULL_SWITCH(OLY_ELEMENT_RAY_HITS);
#undef OLY_ELEMENT_RAY_HITS
	}

	RaycastResult Element::raycast(Ray ray) const
	{
#define OLY_ELEMENT_RAYCAST(p) return col2d::raycast(*p, ray);
		OLY_ELEMENT_IMPL_FULL_SWITCH(OLY_ELEMENT_RAYCAST);
#undef OLY_ELEMENT_RAYCAST
	}

#define OLY_ELEMENT_IMPL_INNER_SWITCH_CASE(Macro, p, Class)\
	case internal::ElementIDTrait<Class>::ID:\
		Macro(p, c.obj.cast<Class>())\
		break;

#define OLY_ELEMENT_IMPL_INNER_SWITCH(Macro, p)\
	switch (c.id)\
	{\
		OLY_ELEMENT_IMPL_INNER_SWITCH_CASE(Macro, p, Circle);\
		OLY_ELEMENT_IMPL_INNER_SWITCH_CASE(Macro, p, AABB);\
		OLY_ELEMENT_IMPL_INNER_SWITCH_CASE(Macro, p, OBB);\
		OLY_ELEMENT_IMPL_INNER_SWITCH_CASE(Macro, p, ConvexHull);\
		OLY_ELEMENT_IMPL_INNER_SWITCH_CASE(Macro, p, KDOP2);\
		OLY_ELEMENT_IMPL_INNER_SWITCH_CASE(Macro, p, KDOP3);\
		OLY_ELEMENT_IMPL_INNER_SWITCH_CASE(Macro, p, KDOP4);\
		OLY_ELEMENT_IMPL_INNER_SWITCH_CASE(Macro, p, KDOP5);\
		OLY_ELEMENT_IMPL_INNER_SWITCH_CASE(Macro, p, KDOP6);\
		OLY_ELEMENT_IMPL_INNER_SWITCH_CASE(Macro, p, KDOP7);\
		OLY_ELEMENT_IMPL_INNER_SWITCH_CASE(Macro, p, KDOP8);\
		default:\
			throw Error(ErrorCode::UNSUPPORTED_SWITCH_CASE);\
	}

	OverlapResult Element::overlaps(const Element& c) const
	{
#define OLY_ELEMENT_OVERLAPS(p1, p2) return col2d::overlaps(*p1, *p2);
#define OLY_ELEMENT_OVERLAPS_INNER(p) OLY_ELEMENT_IMPL_INNER_SWITCH(OLY_ELEMENT_OVERLAPS, p);

		OLY_ELEMENT_IMPL_FULL_SWITCH(OLY_ELEMENT_OVERLAPS_INNER);

#undef OLY_ELEMENT_OVERLAPS_INNER
#undef OLY_ELEMENT_OVERLAPS
	}

	CollisionResult Element::collides(const Element& c) const
	{
#define OLY_ELEMENT_COLLIDES(p1, p2) return col2d::collides(*p1, *p2);
#define OLY_ELEMENT_COLLIDES_INNER(p) OLY_ELEMENT_IMPL_INNER_SWITCH(OLY_ELEMENT_COLLIDES, p);

		OLY_ELEMENT_IMPL_FULL_SWITCH(OLY_ELEMENT_COLLIDES_INNER);

#undef OLY_ELEMENT_COLLIDES_INNER
#undef OLY_ELEMENT_COLLIDES
	}

	ContactResult Element::contacts(const Element& c) const
	{
#define OLY_ELEMENT_CONTACTS(p1, p2) return col2d::contacts(*p1, *p2);
#define OLY_ELEMENT_CONTACTS_INNER(p) OLY_ELEMENT_IMPL_INNER_SWITCH(OLY_ELEMENT_CONTACTS, p);

		OLY_ELEMENT_IMPL_FULL_SWITCH(OLY_ELEMENT_CONTACTS_INNER);

#undef OLY_ELEMENT_CONTACTS_INNER
#undef OLY_ELEMENT_CONTACTS
	}

#undef OLY_ELEMENT_IMPL_INNER_SWITCH
#undef OLY_ELEMENT_IMPL_INNER_SWITCH_CASE
#undef OLY_ELEMENT_IMPL_FULL_SWITCH
#undef OLY_ELEMENT_IMPL_SWITCH_CASE
}
