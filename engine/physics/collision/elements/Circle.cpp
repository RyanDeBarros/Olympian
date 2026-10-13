#include "Circle.h"

#include "core/base/Errors.h"

#include "physics/collision/elements/OBB.h"

namespace oly::col2d
{
    Circle::Circle(glm::vec2 center, float radius)
        : center(center)
        , radius(radius)
    {
    }

	Circle Circle::fast_wrap(const math::Polygon2D& polygon)
	{
		glm::vec2 center = {};
		for (glm::vec2 point : polygon)
			center += point;
		center /= (float)polygon.size();

		float radius = 0.0f;
		for (glm::vec2 point : polygon)
			radius = std::max(radius, math::mag_sqrd(point - center));
		radius = glm::sqrt(radius);

		return Circle(center, radius);
	}

	fpair Circle::projection_interval(const UnitVector2D& axis) const
	{
		if (global_ == glm::mat2(1.0f))
		{
			float center_proj = axis.dot(center + global_offset_);
			return { center_proj - radius, center_proj + radius };
		}
		else
		{
			float offset = axis.dot(global_ * center + global_offset_);
			float multiplier = glm::length(glm::transpose(global_) * axis);
			return { offset - radius * multiplier, offset + radius * multiplier };
		}
	}

	float Circle::projection_min(const UnitVector2D& axis) const
	{
		if (global_ == glm::mat2(1.0f))
			return axis.dot(center + global_offset_) - radius;
		else
			return axis.dot(global_ * center + global_offset_) - radius * glm::length(glm::transpose(global_) * axis);
	}

	float Circle::projection_max(const UnitVector2D& axis) const
	{
		if (global_ == glm::mat2(1.0f))
			return axis.dot(center + global_offset_) + radius;
		else
			return axis.dot(global_ * center + global_offset_) + radius * glm::length(glm::transpose(global_) * axis);
	}

	ContactManifold Circle::deepest_manifold(const UnitVector2D& axis) const
	{
		if (global_ == glm::mat2(1.0f))
			return { .p1 = center + global_offset_ + radius * (glm::vec2)axis };
		else
			return { .p1 = global_ * center + global_offset_ + radius * (glm::vec2)axis * math::inv_magnitude(global_inverse_ * axis) };
	}

    float Circle::area() const
    {
        return glm::pi<float>() * radius * radius * glm::determinant(glm::mat2(global_));
    }

    glm::mat3 Circle::compute_transform() const
    {
        return augment(global_, global_offset_) * translation_matrix(center);
    }

	namespace internal
	{
		const glm::mat2& CircleGlobalAccess::get_global(const Circle& c)
		{
			return c.global_;
		}

		glm::vec2 CircleGlobalAccess::get_global_offset(const Circle& c)
		{
			return c.global_offset_;
		}

		Circle CircleGlobalAccess::create_affine_circle(const Circle& c, const glm::mat3x2& g)
		{
			Circle tc(c.center, c.radius);
			glm::mat3x2 full = augment(g) * augment(c.global_, c.global_offset_);
			tc.global_ = glm::mat2(full);
			tc.global_offset_ = full[2];
			tc.global_inverse_ = glm::inverse(tc.global_);
			return tc;
		}

		bool CircleGlobalAccess::has_no_global(const Circle& c)
		{
			return c.global_ == glm::mat2(1.0f);
		}

		float CircleGlobalAccess::radius_disparity(const Circle& c)
		{
			// DOC
			float mag_sqrd_diff = glm::dot(c.global_[0], c.global_[0]) - glm::dot(c.global_[1], c.global_[1]);
			return glm::sqrt(mag_sqrd_diff * mag_sqrd_diff + 4 * glm::dot(c.global_[0], c.global_[1]) * glm::dot(c.global_[0], c.global_[1]));
		}

		float CircleGlobalAccess::max_radius(const Circle& c)
		{
			// DOC
			return 0.5f * c.radius * (glm::dot(c.global_[0], c.global_[0]) + glm::dot(c.global_[1], c.global_[1]) + radius_disparity(c));
		}

		float CircleGlobalAccess::min_radius(const Circle& c)
		{
			// DOC
			return 0.5f * c.radius * (glm::dot(c.global_[0], c.global_[0]) + glm::dot(c.global_[1], c.global_[1]) - radius_disparity(c));
		}

		Circle CircleGlobalAccess::bounding_circle(const Circle& c)
		{
			// DOC
			return Circle(global_center(c), max_radius(c));
		}

		OBB CircleGlobalAccess::bounding_obb(const Circle& c)
		{
			// DOC
			float max_angle = 0.5f * glm::atan(2.0f * glm::dot(c.global_[0], c.global_[1]), glm::dot(c.global_[0], c.global_[0]) - glm::dot(c.global_[1], c.global_[1]));
			return OBB{ .center = global_center(c), .width = 2.0f * max_radius(c), .height = 2.0f * min_radius(c), .rotation = max_angle };
		}

		glm::vec2 CircleGlobalAccess::global_center(const Circle& c)
		{
			return c.global_ * c.center + c.global_offset_;
		}

		glm::vec2 CircleGlobalAccess::global_point(const Circle& c, glm::vec2 v)
		{
			return c.global_ * v + c.global_offset_;
		}

		glm::vec2 CircleGlobalAccess::global_direction(const Circle& c, glm::vec2 v)
		{
			return c.global_ * v;
		}

		glm::vec2 CircleGlobalAccess::global_normal(const Circle& c, glm::vec2 n)
		{
			return glm::transpose(c.global_inverse_) * n;
		}

		glm::vec2 CircleGlobalAccess::local_point(const Circle& c, glm::vec2 v)
		{
			return c.global_inverse_ * (v - c.global_offset_);
		}

		glm::vec2 CircleGlobalAccess::local_direction(const Circle& c, glm::vec2 v)
		{
			return c.global_inverse_ * v;
		}
	}
}
