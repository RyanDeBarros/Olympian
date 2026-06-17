#include "Types.h"

#include <stdexcept>
#include <string>

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
			throw std::out_of_range(std::to_string(i) + " is invalid index for Color");
	}

	float Color::operator[](size_t i) const
	{
		if (i < N)
			return v[i];
		else
			throw std::out_of_range(std::to_string(i) + " is invalid index for Color");
	}

	Rect::Rect()
		: x1(0.f), x2(1.f), y1(0.f), y2(1.f)
	{
	}

	Rect::Rect(float x1, float x2, float y1, float y2)
		: x1(x1), x2(x2), y1(y1), y2(y2)
	{
	}

	const Rect Rect::ZERO = Rect(0.f, 0.f, 0.f, 0.f);

	bool Rect::operator==(const Rect& o) const
	{
		return x1 == o.x1 && x2 == o.x2 && y1 == o.y1 && y2 == o.y2;
	}

	bool Rect::operator!=(const Rect& o) const
	{
		return x1 != o.x1 || x2 != o.x2 || y1 != o.y1 || y2 != o.y2;
	}

	float* Rect::ValuePtr()
	{
		return v;
	}

	const float* Rect::ValuePtr() const
	{
		return v;
	}

	float& Rect::operator[](size_t i)
	{
		if (i < N)
			return v[i];
		else
			throw std::out_of_range(std::to_string(i) + " is invalid index for Rect");
	}

	float Rect::operator[](size_t i) const
	{
		if (i < N)
			return v[i];
		else
			throw std::out_of_range(std::to_string(i) + " is invalid index for Rect");
	}

	TopSidePadding::TopSidePadding()
		: left(0.f), right(0.f), top(0.f)
	{
	}

	TopSidePadding::TopSidePadding(float left, float right, float top)
		: left(left), right(right), top(top)
	{
	}

	bool TopSidePadding::operator==(const TopSidePadding& o) const
	{
		return v[0] == o.v[0] && v[1] == o.v[1] && v[2] == o.v[2];
	}

	bool TopSidePadding::operator!=(const TopSidePadding& o) const
	{
		return v[0] != o.v[0] || v[1] != o.v[1] || v[2] != o.v[2];
	}

	float* TopSidePadding::ValuePtr()
	{
		return v;
	}

	const float* TopSidePadding::ValuePtr() const
	{
		return v;
	}
	
	float& TopSidePadding::operator[](size_t i)
	{
		if (i < N)
			return v[i];
		else
			throw std::out_of_range(std::to_string(i) + " is invalid index for TopSidePadding");
	}
	
	float TopSidePadding::operator[](size_t i) const
	{
		if (i < N)
			return v[i];
		else
			throw std::out_of_range(std::to_string(i) + " is invalid index for TopSidePadding");
	}
}
