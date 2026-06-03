#pragma once

#include "external/GLM.h"

namespace oly
{
	struct Color
	{
		float r;
		float g;
		float b;
		float a;

		constexpr Color() : r(1.f), g(1.f), b(1.f), a(1.f) {}
		constexpr Color(float r, float g, float b, float a) : r(r), g(g), b(b), a(a) {}
		constexpr Color(glm::vec4 v) : r(v.r), g(v.g), b(v.b), a(v.a) {}
		constexpr operator glm::vec4() const { return { r, g, b, a }; }

		constexpr Color operator*(Color other) const { return Color((glm::vec4)*this * (glm::vec4)other); }
		constexpr bool operator==(Color other) const { return (glm::vec4)*this == (glm::vec4)other; }
		constexpr bool operator!=(Color other) const { return (glm::vec4)*this != (glm::vec4)other; }
	};

	namespace colors
	{
		inline constexpr Color BLACK = { 0.0f, 0.0f, 0.0f, 1.0f };
		inline constexpr Color WHITE = { 1.0f, 1.0f, 1.0f, 1.0f };
		inline constexpr Color RED = { 1.0f, 0.0f, 0.0f, 1.0f };
		inline constexpr Color GREEN = { 0.0f, 1.0f, 0.0f, 1.0f };
		inline constexpr Color BLUE = { 0.0f, 0.0f, 1.0f, 1.0f };
		inline constexpr Color YELLOW = { 1.0f, 1.0f, 0.0f, 1.0f };
		inline constexpr Color MAGENTA = { 1.0f, 0.0f, 1.0f, 1.0f };
		inline constexpr Color CYAN = { 0.0f, 1.0f, 1.0f, 1.0f };
		inline constexpr Color ORANGE = { 1.0f, 0.5f, 0.0f, 1.0f };

		inline constexpr Color alpha(float a) { return { 1.0f, 1.0f, 1.0f, a }; }
	}
}
