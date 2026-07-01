#include "GamepadAxis2D.h"

namespace oly::detail
{
	std::ostream& operator<<(std::ostream& os, GamepadAxis2D axis)
	{
		os << "GamepadAxis2D(";
		
		switch (axis)
		{
		case GamepadAxis2D::LeftXY:
			os << "LeftXY";
			break;

		case GamepadAxis2D::RightXY:
			os << "RightXY";
			break;

		default:
			os << "unknown";
			break;
		}

		return os << ")";
	}
}
