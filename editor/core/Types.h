#pragma once

#include "external/GLM.h"

namespace oly::editor
{
	struct Color4
	{
		static inline const size_t N = 4;

		union
		{
			struct { float r, g, b, a; };
			float v[N];
		};

		Color4();
		Color4(float r, float g, float b, float a);

		bool operator==(const Color4&) const;
		bool operator!=(const Color4&) const;

		float* ValuePtr();
		const float* ValuePtr() const;
		float& operator[](size_t i);
		float operator[](size_t i) const;
	};

	struct Rect
	{
		static inline const size_t N = 4;

		union
		{
			struct { float x1, x2, y1, y2; };
			float v[N];
		};

		Rect();
		Rect(float x1, float x2, float y1, float y2);

		bool operator==(const Rect&) const;
		bool operator!=(const Rect&) const;

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
			float v[N];
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

	struct TopSidePadding
	{
		static inline const size_t N = 3;

		union
		{
			struct { float left, right, top; };
			float v[N];
		};

		TopSidePadding();
		TopSidePadding(float left, float right, float top);

		bool operator==(const TopSidePadding&) const;
		bool operator!=(const TopSidePadding&) const;

		float* ValuePtr();
		const float* ValuePtr() const;
		float& operator[](size_t i);
		float operator[](size_t i) const;
	};
}
