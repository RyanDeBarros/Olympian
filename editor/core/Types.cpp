#include "Types.h"

#include <stdexcept>
#include <string>

namespace oly::editor
{
	Color4::Color4()
		: r(0.f), g(0.f), b(0.f), a(1.f)
	{
	}

	Color4::Color4(float r, float g, float b, float a)
		: r(r), g(g), b(b), a(a)
	{
	}

	bool Color4::operator==(const Color4& o) const
	{
		return r == o.r && g == o.g && b == o.b && a == o.a;
	}

	bool Color4::operator!=(const Color4& o) const
	{
		return r != o.r || g != o.g || b != o.b || a != o.a;
	}

	float* Color4::ValuePtr()
	{
		return v;
	}

	const float* Color4::ValuePtr() const
	{
		return v;
	}

	float& Color4::operator[](size_t i)
	{
		if (i < N)
			return v[i];
		else
			throw std::out_of_range(std::to_string(i) + " is invalid index for Color4");
	}

	float Color4::operator[](size_t i) const
	{
		if (i < N)
			return v[i];
		else
			throw std::out_of_range(std::to_string(i) + " is invalid index for Color4");
	}

	std::ostream& operator<<(std::ostream& os, Color4 color)
	{
		return os << "Color4(" << color.r << ", " << color.g << ", " << color.b << ", " << color.a << ")";
	}

	Rect::Rect()
		: x1(0.f), x2(0.f), y1(0.f), y2(0.f)
	{
	}

	Rect::Rect(float x1, float x2, float y1, float y2)
		: x1(x1), x2(x2), y1(y1), y2(y2)
	{
	}

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

	std::ostream& operator<<(std::ostream& os, Rect rect)
	{
		return os << "Rect([" << rect.x1 << ", " << rect.y1 << "], [" << rect.x2 << ", " << rect.y2 << "])";
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
			throw std::out_of_range(std::to_string(i) + " is invalid index for UVRect");
	}

	float UVRect::operator[](size_t i) const
	{
		if (i < N)
			return v[i];
		else
			throw std::out_of_range(std::to_string(i) + " is invalid index for UVRect");
	}

	std::ostream& operator<<(std::ostream& os, UVRect uv_rect)
	{
		return os << "UVRect([" << uv_rect.x1 << ", " << uv_rect.y1 << "], [" << uv_rect.x2 << ", " << uv_rect.y2 << "])";
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

	std::ostream& operator<<(std::ostream& os, TopSidePadding padding)
	{
		return os << "TopSidePadding(left=" << padding.left << ", right=" << padding.right << ", top=" << padding.top << ")";
	}
}
