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
}
