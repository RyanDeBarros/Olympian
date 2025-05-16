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

	OverlapInfo overlap(const std::vector<glm::vec2>& c1, const std::vector<glm::vec2>& c2)
	{
		for (size_t i = 0; i < c1.size(); ++i)
		{
			glm::vec2 p1 = c1[i];
			glm::vec2 p2 = c1[(i + 1) % c1.size()];
			if (sat(p1, p2, c1, c2) < 0.0f)
				return false;
		}

		for (size_t i = 0; i < c2.size(); ++i)
		{
			glm::vec2 p1 = c2[i];
			glm::vec2 p2 = c2[(i + 1) % c2.size()];
			if (sat(p1, p2, c1, c2) < 0.0f)
				return false;
		}

		return true;
	}

	GeometricInfo geometric_collision(const std::vector<glm::vec2>& c1, const std::vector<glm::vec2>& c2)
	{
		GeometricInfo info{};

		static const auto update_info = [](GeometricInfo& info, glm::vec2 p1, glm::vec2 p2, const std::vector<glm::vec2>&c1, const std::vector<glm::vec2>&c2)
			{
				float depth = sat(p1, p2, c1, c2);
				if (depth < info.penetration_depth)
				{
					info.penetration_depth = depth;
					glm::vec2 edge = p2 - p1;
					info.unit_impulse = glm::normalize(glm::vec2{ -edge.y, edge.x });
				}
			};

		info.penetration_depth = FLT_MAX;

		for (size_t i = 0; i < c1.size(); ++i)
		{
			glm::vec2 p1 = c1[i];
			glm::vec2 p2 = c1[(i + 1) % c1.size()];
			update_info(info, p1, p2, c1, c2);
			if (info.penetration_depth < 0.0f)
				return { .overlap = false };
		}

		for (size_t i = 0; i < c2.size(); ++i)
		{
			glm::vec2 p1 = c2[i];
			glm::vec2 p2 = c2[(i + 1) % c2.size()];
			update_info(info, p1, p2, c1, c2);
			if (info.penetration_depth < 0.0f)
				return { .overlap = false };
		}

		info.overlap = true;
		return info;
	}

	OverlapInfo overlap(const Circle& c1, const std::vector<glm::vec2>& c2)
	{
		float closest_dist_sqrd = FLT_MAX;
		glm::vec2 closest_point{};

		for (size_t i = 0; i < c2.size(); ++i)
		{
			glm::vec2 p1 = c2[i];
			glm::vec2 p2 = c2[(i + 1) % c2.size()];
			if (sat(p1, p2, c1, c2) < 0.0f)
				return false;

			float dist_sqrd = math::mag_sqrd(c1.center - p1);
			if (dist_sqrd < closest_dist_sqrd)
			{
				closest_dist_sqrd = dist_sqrd;
				closest_point = p1;
			}
		}

		// circle center to closest point
		if (closest_point != c1.center)
		{
			glm::vec2 axis = glm::normalize(closest_point - c1.center);
			std::pair<float, float> i1 = projection_interval(c1, axis);
			std::pair<float, float> i2 = projection_interval(c2, axis);
			float length = axis_signed_overlap(i1.first, i1.second, i2.first, i2.second);
			if (length < 0.0f)
				return false;
		}

		return true;
	}
	
	GeometricInfo geometric_collision(const Circle& c1, const std::vector<glm::vec2>& c2)
	{
		GeometricInfo info{};

		static const auto update_info = [](GeometricInfo& info, glm::vec2 p1, glm::vec2 p2, const Circle& c1, const std::vector<glm::vec2>& c2)
			{
				float depth = sat(p1, p2, c1, c2);
				if (depth < info.penetration_depth)
				{
					info.penetration_depth = depth;
					glm::vec2 edge = p2 - p1;
					info.unit_impulse = glm::normalize(glm::vec2{ -edge.y, edge.x });
				}
			};

		info.penetration_depth = FLT_MAX;

		float closest_dist_sqrd = FLT_MAX;
		glm::vec2 closest_point{};

		for (size_t i = 0; i < c2.size(); ++i)
		{
			glm::vec2 p1 = c2[i];
			glm::vec2 p2 = c2[(i + 1) % c2.size()];
			update_info(info, p1, p2, c1, c2);
			if (info.penetration_depth < 0.0f)
				return { .overlap = false };

			float dist_sqrd = math::mag_sqrd(c1.center - p1);
			if (dist_sqrd < closest_dist_sqrd)
			{
				closest_dist_sqrd = dist_sqrd;
				closest_point = p1;
			}
		}

		// circle center to closest point
		if (closest_point != c1.center)
		{
			glm::vec2 axis = glm::normalize(closest_point - c1.center);
			std::pair<float, float> i1 = projection_interval(c1, axis);
			std::pair<float, float> i2 = projection_interval(c2, axis);
			float length = axis_signed_overlap(i1.first, i1.second, i2.first, i2.second);
			if (length < 0.0f)
				return { .overlap = false };
			else if (length < info.penetration_depth)
			{
				info.penetration_depth = length;
				info.unit_impulse = axis;
			}
		}

		info.overlap = true;
		return info;
	}
}
