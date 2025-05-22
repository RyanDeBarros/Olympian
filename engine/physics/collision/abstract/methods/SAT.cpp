#include "SAT.h"

// LATER edge normals can be cached
namespace oly::acm2d::sat
{
	static std::pair<float, float> projection_interval(const std::vector<glm::vec2>& polygon, glm::vec2 axis)
	{
		std::pair<float, float> interval = { FLT_MAX, -FLT_MAX };
		for (glm::vec2 point : polygon)
		{
			float proj = glm::dot(point, axis);
			interval.first = std::min(interval.first, proj);
			interval.second = std::max(interval.second, proj);
		}
		return interval;
	}

	static std::pair<float, float> projection_interval(const Circle& circle, glm::vec2 axis)
	{
		float center_proj = glm::dot(circle.center, axis);
		return { center_proj - circle.radius, center_proj + circle.radius };
	}

	static float axis_signed_overlap(float min1, float max1, float min2, float max2)
	{
		return std::min(max1, max2) - std::max(min1, min2);
	}

	template<typename C1, typename C2>
	static float sat(glm::vec2 p1, glm::vec2 p2, const C1& c1, const C2& c2)
	{
		glm::vec2 edge = p2 - p1;
		glm::vec2 normal = glm::normalize(glm::vec2{ -edge.y, edge.x });
		std::pair<float, float> i1 = projection_interval(c1, normal);
		std::pair<float, float> i2 = projection_interval(c2, normal);
		return axis_signed_overlap(i1.first, i1.second, i2.first, i2.second);
	};

	template<typename C>
	static float sat(const std::vector<glm::vec2>& c1, const C& c2, size_t i)
	{
		glm::vec2 p1 = c1[i];
		glm::vec2 p2 = c1[(i + 1) % c1.size()];
		return sat(p1, p2, c1, c2);
	}

	template<typename C1, typename C2>
	static float sat(const C1& c1, const C2& c2, glm::vec2 axis)
	{
		std::pair<float, float> i1 = projection_interval(c1, axis);
		std::pair<float, float> i2 = projection_interval(c2, axis);
		return axis_signed_overlap(i1.first, i1.second, i2.first, i2.second);
	}

	template<typename Other>
	static OverlapResult overlaps_impl(const std::vector<glm::vec2>& c, const Other& other)
	{
		for (size_t i = 0; i < c.size(); ++i)
			if (sat(c, other, i) < 0.0f)
				return false;

		return true;
	}

	OverlapResult overlaps(const std::vector<glm::vec2>& c1, const std::vector<glm::vec2>& c2)
	{
		return overlaps_impl(c1, c2) && overlaps_impl(c2, c1);
	}

	template<typename C1, typename C2>
	static void update_collision_result(CollisionResult& info, glm::vec2 p1, glm::vec2 p2, const C1& c1, const C2& c2)
	{
		float depth = sat(p1, p2, c1, c2);
		if (depth < info.penetration_depth)
		{
			info.penetration_depth = depth;
			glm::vec2 edge = p2 - p1;
			info.unit_impulse = glm::normalize(glm::vec2{ -edge.y, edge.x });
		}
	};

	template<typename C>
	static void update_collision_result(CollisionResult& info, const std::vector<glm::vec2>& c1, const C& c2, size_t i)
	{
		update_collision_result(info, c1[i], c1[(i + 1) % c1.size()], c1, c2);
	}

	template<typename Other>
	static void update_collision_result_impl(CollisionResult& info, const std::vector<glm::vec2>& c, const Other& other)
	{
		for (size_t i = 0; i < c.size(); ++i)
		{
			update_collision_result(info, c, other, i);
			if (info.penetration_depth < 0.0f)
			{
				info.overlap = false;
				info.penetration_depth = 0.0f;
				return;
			}
		}
	}

	CollisionResult collides(const std::vector<glm::vec2>& c1, const std::vector<glm::vec2>& c2)
	{
		CollisionResult info{ .overlap = true, .penetration_depth = FLT_MAX };
		update_collision_result_impl(info, c1, c2);
		if (!info.overlap)
			return info;
		update_collision_result_impl(info, c2, c1);
		if (!info.overlap)
			return info;
		return info;
	}

	OverlapResult overlaps(const Circle& c1, const std::vector<glm::vec2>& c2)
	{
		float closest_dist_sqrd = FLT_MAX;
		glm::vec2 closest_point{};

		for (size_t i = 0; i < c2.size(); ++i)
		{
			if (sat(c2, c1, i) < 0.0f)
				return false;

			float dist_sqrd = math::mag_sqrd(c1.center - c2[i]);
			if (dist_sqrd < closest_dist_sqrd)
			{
				closest_dist_sqrd = dist_sqrd;
				closest_point = c2[i];
			}
		}

		// circle center to closest point
		if (closest_point != c1.center && sat(c1, c2, glm::normalize(closest_point - c1.center)) < 0.0f)
			return false;

		return true;
	}
	
	CollisionResult collides(const Circle& c1, const std::vector<glm::vec2>& c2)
	{
		CollisionResult info{ .overlap = true, .penetration_depth = FLT_MAX };

		float closest_dist_sqrd = FLT_MAX;
		glm::vec2 closest_point{};

		for (size_t i = 0; i < c2.size(); ++i)
		{
			update_collision_result(info, c2, c1, i);
			if (info.penetration_depth < 0.0f)
				return { .overlap = false };

			float dist_sqrd = math::mag_sqrd(c1.center - c2[i]);
			if (dist_sqrd < closest_dist_sqrd)
			{
				closest_dist_sqrd = dist_sqrd;
				closest_point = c2[i];
			}
		}

		// circle center to closest point
		if (closest_point != c1.center)
		{
			glm::vec2 axis = glm::normalize(closest_point - c1.center);
			float length = sat(c1, c2, axis);
			if (length < 0.0f)
				return { .overlap = false };
			else if (length < info.penetration_depth)
			{
				info.penetration_depth = length;
				info.unit_impulse = axis;
			}
		}

		return info;
	}

	static std::pair<float, float> projection_interval(const AABB& aabb, glm::vec2 axis)
	{
		std::pair<float, float> interval = { FLT_MAX, -FLT_MAX };
		glm::vec2 points[4] = {
			{ aabb.x1, aabb.y1 },
			{ aabb.x2, aabb.y1 },
			{ aabb.x2, aabb.y2 },
			{ aabb.x1, aabb.y2 }
		};
		for (glm::vec2 point : points)
		{
			float proj = glm::dot(point, axis);
			interval.first = std::min(interval.first, proj);
			interval.second = std::max(interval.second, proj);
		}
		return interval;
	}

	template<typename Other>
	static OverlapResult overlaps_impl(const AABB& c, const Other& other)
	{
		return sat(c, other, { 1.0f, 0.0f }) >= 0.0f && sat(c, other, { 0.0f, 1.0f }) >= 0.0f;
	}

	OverlapResult overlaps(const AABB& c1, const std::vector<glm::vec2>& c2)
	{
		return overlaps_impl(c1, c2) && overlaps_impl(c2, c1);
	}

	template<typename Other>
	static void update_collision_result_impl(CollisionResult& info, const AABB& c, const Other& other)
	{
		{
			float depth = sat(c, other, { 1.0f, 0.0f });
			if (depth < 0.0f)
			{
				info.overlap = false;
				return;
			}
			else if (depth < info.penetration_depth)
			{
				info.penetration_depth = depth;
				info.unit_impulse = { 0.0f, 1.0f };
			}
		}

		{
			float depth = sat(c, other, { 0.0f, 1.0f });
			if (depth < 0.0f)
			{
				info.overlap = false;
				return;
			}
			else if (depth < info.penetration_depth)
			{
				info.penetration_depth = depth;
				info.unit_impulse = { -1.0f, 0.0f }; // TODO should this be { 1, 0 } in some cases? Test SAT extensively.
			}
		}
	}

	CollisionResult collides(const AABB& c1, const std::vector<glm::vec2>& c2)
	{
		CollisionResult info{ .overlap = true, .penetration_depth = FLT_MAX };
		update_collision_result_impl(info, c1, c2);
		if (!info.overlap)
			return info;
		update_collision_result_impl(info, c2, c1);
		if (!info.overlap)
			return info;
		return info;
	}

	static std::pair<float, float> projection_interval(const OBB& obb, glm::vec2 axis)
	{
		std::pair<float, float> interval = { FLT_MAX, -FLT_MAX };
		glm::vec2 points[4] = {
			{ -0.5f * obb.width, -0.5f * obb.height },
			{  0.5f * obb.width, -0.5f * obb.height },
			{  0.5f * obb.width,  0.5f * obb.height },
			{ -0.5f * obb.width,  0.5f * obb.height }
		};

		glm::mat2 rotation = obb.get_rotation_matrix();
		for (glm::vec2 point : points)
		{
			point = obb.center + rotation * point;
			float proj = glm::dot(point, axis);
			interval.first = std::min(interval.first, proj);
			interval.second = std::max(interval.second, proj);
		}
		return interval;
	}

	template<typename Other>
	static OverlapResult overlaps_impl(const OBB& c, const Other& other)
	{
		std::array<glm::vec2, 2> axes = c.get_axes();

		{
			float cw = glm::dot(c.center, axes[0]);
			std::pair<float, float> i1 = { cw - 0.5f * c.width, cw + 0.5f * c.width };
			std::pair<float, float> i2 = projection_interval(other, axes[0]);
			if (axis_signed_overlap(i1.first, i1.second, i2.first, i2.second) < 0.0f)
				return false;
		}

		{
			float ch = glm::dot(c.center, axes[1]);
			std::pair<float, float> i1 = { ch - 0.5f * c.height, ch + 0.5f * c.height };
			std::pair<float, float> i2 = projection_interval(other, axes[1]);
			if (axis_signed_overlap(i1.first, i1.second, i2.first, i2.second) < 0.0f)
				return false;
		}

		return true;
	}

	OverlapResult overlaps(const OBB& c1, const std::vector<glm::vec2>& c2)
	{
		return overlaps_impl(c1, c2) && overlaps_impl(c2, c1);
	}

	template<typename Other>
	static void update_collision_result_impl(CollisionResult& info, const OBB& c, const Other& other)
	{
		std::array<glm::vec2, 2> axes = c.get_axes();

		{
			float depth = sat(c, other, axes[0]);
			if (depth < 0.0f)
			{
				info.overlap = false;
				return;
			}
			else if (depth < info.penetration_depth)
			{
				info.penetration_depth = depth;
				info.unit_impulse = { -axes[1].y, axes[1].x };
			}
		}

		{
			float depth = sat(c, other, axes[1]);
			if (depth < 0.0f)
			{
				info.overlap = false;
				return;
			}
			else if (depth < info.penetration_depth)
			{
				info.penetration_depth = depth;
				info.unit_impulse = { -axes[1].y, axes[1].x };
			}
		}
	}

	CollisionResult collides(const OBB& c1, const std::vector<glm::vec2>& c2)
	{
		CollisionResult info{ .overlap = true, .penetration_depth = FLT_MAX };
		update_collision_result_impl(info, c1, c2);
		if (!info.overlap)
			return info;
		update_collision_result_impl(info, c2, c1);
		if (!info.overlap)
			return info;
		return info;
	}
}
