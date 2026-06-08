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
}
