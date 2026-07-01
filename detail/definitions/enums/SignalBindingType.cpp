#include "SignalBindingType.h"

namespace oly::detail
{
	std::ostream& operator<<(std::ostream& os, SignalBindingType type)
	{
		os << "SignalBindingType(";

		switch (type)
		{
		case SignalBindingType::Key:
			os << "Key";
			break;

		case SignalBindingType::MouseButton:
			os << "MouseButton";
			break;

		case SignalBindingType::GamepadButton:
			os << "GamepadButton";
			break;

		case SignalBindingType::GamepadAxis1D:
			os << "GamepadAxis1D";
			break;

		case SignalBindingType::GamepadAxis2D:
			os << "GamepadAxis2D";
			break;

		case SignalBindingType::CursorPos:
			os << "CursorPos";
			break;

		case SignalBindingType::Scroll:
			os << "Scroll";
			break;

		default:
			os << "unknown";
			break;
		}

		return os << ")";
	}
}
