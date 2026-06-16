#pragma once

#include "external/GLM.h"

namespace oly::editor
{
	struct Color
	{
		static inline const size_t N = 4;

		union
		{
			struct { float r, g, b, a; };
			float v[4];
		};

		Color();
		Color(float r, float g, float b, float a);

		bool operator==(const Color&) const;
		bool operator!=(const Color&) const;

		float* ValuePtr();
		const float* ValuePtr() const;
		float& operator[](size_t i);
		float operator[](size_t i) const;
	};

	struct UVRect
	{
		static inline const size_t N = 4;

		union
		{
			struct { float x1, x2, y1, y2; };
			float v[4];
		};

		UVRect();
		UVRect(float x1, float x2, float y1, float y2);

		bool operator==(const UVRect&) const;
		bool operator!=(const UVRect&) const;

		float* ValuePtr();
		const float* ValuePtr() const;
		float& operator[](size_t i);
		float operator[](size_t i) const;
	};
}
