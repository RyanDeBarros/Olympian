#include "Element.h"

#include "core/base/Transforms.h"

namespace oly::col2d
{
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
			std::vector<UnitVector2D> axes(2);
			std::vector<float> minima(2);
			std::vector<float> maxima(2);

			{
				glm::vec2 normal = transform_normal(m, UnitVector2D::RIGHT);
				axes[0] = UnitVector2D(normal);
				float normalization = math::inv_magnitude(normal);
				float offset = axes[0].dot(m[2]);
				minima[0] = offset + c.x1 * normalization;
				maxima[0] = offset + c.x2 * normalization;
			}

			{
				glm::vec2 normal = transform_normal(m, UnitVector2D::UP);
				axes[1] = UnitVector2D(normal);
				float normalization = math::inv_magnitude(normal);
				float offset = axes[1].dot(m[2]);
				minima[1] = offset + c.y1 * normalization;
				maxima[1] = offset + c.y2 * normalization;
			}

			return make_copy_ptr<CustomKDOP>(std::move(axes), std::move(minima), std::move(maxima));
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
			std::vector<UnitVector2D> axes(2);
			std::vector<float> minima(2);
			std::vector<float> maxima(2);

			{
				glm::vec2 normal = transform_normal(m, c.get_major_axis());
				axes[0] = UnitVector2D(normal);
				float normalization = math::inv_magnitude(normal);
				float offset = axes[1].dot(m[2]);
				float local_offset = c.get_major_axis().dot(c.center);
				minima[0] = offset + (local_offset - 0.5f * c.width) * normalization;
				maxima[0] = offset + (local_offset + 0.5f * c.width) * normalization;
			}

			{
				glm::vec2 normal = transform_normal(m, c.get_minor_axis());
				axes[1] = UnitVector2D(normal);
				float normalization = math::inv_magnitude(normal);
				float offset = axes[1].dot(m[2]);
				float local_offset = c.get_minor_axis().dot(c.center);
				minima[1] = offset + (local_offset - 0.5f * c.height) * normalization;
				maxima[1] = offset + (local_offset + 0.5f * c.height) * normalization;
			}

			return make_copy_ptr<CustomKDOP>(std::move(axes), std::move(minima), std::move(maxima));
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
			glm::vec2 normal = transform_normal(m, c->edge_normal(i));
			axes[i] = UnitVector2D(normal);
			float normalization = math::inv_magnitude(normal);
			float offset = axes[i].dot(m[2]);
			minima[i] = offset + c->get_minimum(i) * normalization;
			maxima[i] = offset + c->get_maximum(i) * normalization;
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
				glm::vec2 normal = transform_normal(m, KDOP<K>::uniform_axis(i));
				axes[i] = UnitVector2D(normal);
				float normalization = math::inv_magnitude(normal);
				float offset = axes[i].dot(m[2]);
				minima[i] = offset + c->get_minimum(i) * normalization;
				maxima[i] = offset + c->get_maximum(i) * normalization;
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
		const std::vector<glm::vec2>& points = c.points();
		std::vector<glm::vec2>& tpoints = tc.set_points();
		tpoints.reserve(points.size());
		for (glm::vec2 p : points)
			tpoints.push_back(transform_point(m, p));
		return tc;
	}
}
