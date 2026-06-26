#include "AxisConversions.h"

namespace oly::detail
{
	std::ostream& operator<<(std::ostream& os, Swizzle swizzle)
	{
		os << "Swizzle(";

		switch (swizzle)
		{
		case Swizzle::None:
			os << "None";
			break;

		case Swizzle::YX:
			os << "YX";
			break;

		case Swizzle::XZY:
			os << "XZY";
			break;

		case Swizzle::YXZ:
			os << "YXZ";
			break;

		case Swizzle::YZX:
			os << "YZX";
			break;

		case Swizzle::ZXY:
			os << "ZXY";
			break;

		case Swizzle::ZYX:
			os << "ZYX";
			break;

		default:
			os << "unknown";
			break;
		}

		return os << ")";
	}

	std::ostream& operator<<(std::ostream& os, Axis0dConversion conversion)
	{
		os << "Axis0dConversion(";

		switch (conversion)
		{
		case Axis0dConversion::None:
			os << "None";
			break;

		case Axis0dConversion::To1D:
			os << "To1D";
			break;
		
		case Axis0dConversion::To2D:
			os << "To2D";
			break;
		
		case Axis0dConversion::To3D:
			os << "To3D";
			break;

		default:
			os << "unknown";
			break;
		}

		return os << ")";
	}

	std::ostream& operator<<(std::ostream& os, Axis1dConversion conversion)
	{
		os << "Axis1dConversion(";

		switch (conversion)
		{
		case Axis1dConversion::None:
			os << "None";
			break;

		case Axis1dConversion::To0D:
			os << "To0D";
			break;

		case Axis1dConversion::To2D:
			os << "To2D";
			break;

		case Axis1dConversion::To3D:
			os << "To3D";
			break;

		default:
			os << "unknown";
			break;
		}

		return os << ")";
	}

	std::ostream& operator<<(std::ostream& os, Axis2dConversion conversion)
	{
		os << "Axis2dConversion(";

		switch (conversion)
		{
		case Axis2dConversion::None:
			os << "None";
			break;

		case Axis2dConversion::To0D_X:
			os << "To0D_X";
			break;

		case Axis2dConversion::To0D_Y:
			os << "To0D_Y";
			break;

		case Axis2dConversion::To0D_XY:
			os << "To0D_XY";
			break;

		case Axis2dConversion::To1D_X:
			os << "To1D_X";
			break;

		case Axis2dConversion::To1D_Y:
			os << "To1D_Y";
			break;

		case Axis2dConversion::To1D_XY:
			os << "To1D_XY";
			break;

		case Axis2dConversion::To3D_0:
			os << "To3D_0";
			break;

		case Axis2dConversion::To3D_1:
			os << "To3D_1";
			break;

		default:
			os << "unknown";
			break;
		}

		return os << ")";
	}
}
