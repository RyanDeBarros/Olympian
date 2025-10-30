#pragma once

#include "external/GLM.h"

#include <array>

namespace oly::math
{
	enum class PositioningMode
	{
		ABSOLUTE,
		RELATIVE
	};

	struct Triangle2D
	{
		glm::vec2 root;
		glm::vec2 prev;
		glm::vec2 next;

		glm::vec2 dprev() const { return root - prev; }
		glm::vec2 dnext() const { return next - root; }
		float signed_area() const;
		float area() const { return std::abs(signed_area()); }
		float cross() const;
	};

	struct DirectedLine2D
	{
		glm::vec2 anchor;
		glm::vec2 dir;

		bool intersect(const DirectedLine2D& other, glm::vec2 pt) const;
	};

	struct Rect2D
	{
		float x1 = 0.0f, x2 = 0.0f, y1 = 0.0f, y2 = 0.0f;

		float center_x() const { return 0.5f * (x1 + x2); }
		float center_y() const { return 0.5f * (y1 + y2); }
		glm::vec2 center() const { return 0.5f * glm::vec2{ x1 + x2, y1 + y2 }; }
		float width() const { return x2 - x1; }
		float height() const { return y2 - y1; }
		glm::vec2 size() const { return { x2 - x1, y2 - y1 }; }
		
		std::array<glm::vec2, 4> uvs() const
		{
			return {
				glm::vec2{ x1, y1 },
				glm::vec2{ x2, y1 },
				glm::vec2{ x2, y2 },
				glm::vec2{ x1, y2 }
			};
		}

		bool contains(glm::vec2 test) const { return test.x >= x1 && test.x <= x2 && test.y >= y1 && test.y <= y2; }
		bool inside(const Rect2D& enclosing) const { return x1 >= enclosing.x1 && x2 <= enclosing.x2 && y1 >= enclosing.y1 && y2 <= enclosing.y2; }
		bool strict_inside(const Rect2D& enclosing) const { return x1 > enclosing.x1 && x2 < enclosing.x2 && y1 > enclosing.y1 && y2 < enclosing.y2; }
		bool overlaps(const Rect2D& other) const { return x1 <= other.x2 && other.x1 <= x2 && y1 <= other.y2 && other.y1 <= y2; }
		glm::vec2 clamp(glm::vec2 pt) const { return { glm::clamp(pt.x, x1, x2), glm::clamp(pt.y, y1, y2) }; }

		void include(glm::vec2 pt) { x1 = glm::min(x1, pt.x); x2 = glm::max(x2, pt.x); y1 = glm::min(y1, pt.y); y2 = glm::max(y2, pt.y); }

		bool operator==(const Rect2D&) const = default;

		Rect2D get_scaled(float sx, float sy) const { return { .x1 = x1 * sx, .x2 = x2 * sx, .y1 = y1 * sy, .y2 = y2 * sy }; }
		Rect2D get_scaled(glm::vec2 sc) const { return get_scaled(sc.x, sc.y); }
	};

	struct IRect2D
	{
		int x1, x2, y1, y2;

		float center_x() const { return 0.5f * (x1 + x2); }
		float center_y() const { return 0.5f * (y1 + y2); }
		glm::vec2 center() const { return 0.5f * glm::vec2{ x1 + x2, y1 + y2 }; }
		bool contains(glm::ivec2 test) const { return test.x >= x1 && test.x <= x2 && test.y >= y1 && test.y <= y2; }
		glm::ivec2 clamp(glm::ivec2 pt) const { return { glm::clamp(pt.x, x1, x2), glm::clamp(pt.y, y1, y2) }; }
		int width() const { return x2 - x1; }
		int height() const { return y2 - y1; }
		glm::ivec2 size() const { return { x2 - x1, y2 - y1 }; }

		bool operator==(const IRect2D&) const = default;
		explicit operator Rect2D() const { return Rect2D{ .x1 = (float)x1, .x2 = (float)x2, .y1 = (float)y1, .y2 = (float)y2 }; }
	};

	struct UVRect
	{
		float x1 = 0.0f, x2 = 1.0f, y1 = 0.0f, y2 = 1.0f;

		float interp_x(float alpha) const { return (x2 - x1) * alpha + x1; }
		float interp_y(float alpha) const { return (y2 - y1) * alpha + y1; }

		static UVRect from_grid(unsigned int row, unsigned int col, unsigned int rows, unsigned int cols)
		{
			return { .x1 = col / (float)cols, .x2 = (col + 1) / (float)cols, .y1 = row / (float)rows, .y2 = (row + 1) / (float)rows };
		}

		bool operator==(const UVRect&) const = default;
	};

	struct Area2D
	{
		float x = 0.0f, y = 0.0f, w = 1.0f, h = 1.0f;

		glm::vec2 center() const
		{
			return glm::vec2{ x + 0.5f * w, y + 0.5f * h };
		}

		glm::vec2 size() const
		{
			return { w, h };
		}
	};

	struct IArea2D
	{
		int x = 0, y = 0, w = 1, h = 1;
	};

	struct IArea3D
	{
		int x = 0, y = 0, z = 0, w = 1, h = 1, d = 1;
	};

	struct Padding
	{
		float left = 0.0f, right = 0.0f, bottom = 0.0f, top = 0.0f;

		static Padding uniform(float padding)
		{
			return {
				.left = padding,
				.right = padding,
				.bottom = padding,
				.top = padding
			};
		}
	};

	struct TopSidePadding
	{
		float left = 0.0f, right = 0.0f, top = 0.0f;

		static TopSidePadding uniform(float padding)
		{
			return {
				.left = padding,
				.right = padding,
				.top = padding
			};
		}
	};
}

template<>
struct std::hash<oly::math::Rect2D>
{
	size_t operator()(oly::math::Rect2D r) const
	{
		return std::hash<float>{}(r.x1) ^ (std::hash<float>{}(r.x2) << 1) ^ (std::hash<float>{}(r.y1) << 2)
			^ (std::hash<float>{}(r.y2) << 3);
	}
};

template<>
struct std::hash<oly::math::IRect2D>
{
	size_t operator()(oly::math::IRect2D r) const
	{
		return std::hash<int>{}(r.x1) ^ (std::hash<int>{}(r.x2) << 1) ^ (std::hash<int>{}(r.y1) << 2)
			^ (std::hash<int>{}(r.y2) << 3);
	}
};

template<>
struct std::hash<oly::math::UVRect>
{
	size_t operator()(oly::math::UVRect r) const
	{
		return std::hash<float>{}(r.x1) ^ (std::hash<float>{}(r.x2) << 1) ^ (std::hash<float>{}(r.y1) << 2)
			^ (std::hash<float>{}(r.y2) << 3);
	}
};
