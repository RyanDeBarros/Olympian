#include "Types.h"

#include <stdexcept>

namespace oly::editor
{
	Color::Color()
		: r(0.f), g(0.f), b(0.f), a(1.f)
	{
	}

	Color::Color(float r, float g, float b, float a)
		: r(r), g(g), b(b), a(a)
	{
	}

	bool Color::operator==(const Color& o) const
	{
		return r == o.r && g == o.g && b == o.b && a == o.a;
	}

	bool Color::operator!=(const Color& o) const
	{
		return r != o.r || g != o.g || b != o.b || a != o.a;
	}

	float* Color::ValuePtr()
	{
		return v;
	}

	const float* Color::ValuePtr() const
	{
		return v;
	}

	float& Color::operator[](size_t i)
	{
		if (i < N)
			return v[i];
		else
			throw std::out_of_range(i + " is invalid index for Color");
	}

	float Color::operator[](size_t i) const
	{
		if (i < N)
			return v[i];
		else
			throw std::out_of_range(i + " is invalid index for Color");
	}

	UVRect::UVRect()
		: x1(0.f), x2(1.f), y1(0.f), y2(1.f)
	{
	}

	UVRect::UVRect(float x1, float x2, float y1, float y2)
		: x1(x1), x2(x2), y1(y1), y2(y2)
	{
	}

	bool UVRect::operator==(const UVRect& o) const
	{
		return x1 == o.x1 && x2 == o.x2 && y1 == o.y1 && y2 == o.y2;
	}

	bool UVRect::operator!=(const UVRect& o) const
	{
		return x1 != o.x1 || x2 != o.x2 || y1 != o.y1 || y2 != o.y2;
	}

	float* UVRect::ValuePtr()
	{
		return v;
	}

	const float* UVRect::ValuePtr() const
	{
		return v;
	}

	float& UVRect::operator[](size_t i)
	{
		if (i < N)
			return v[i];
		else
			throw std::out_of_range(i + " is invalid index for UVRect");
	}

	float UVRect::operator[](size_t i) const
	{
		if (i < N)
			return v[i];
		else
			throw std::out_of_range(i + " is invalid index for UVRect");
	}
}
