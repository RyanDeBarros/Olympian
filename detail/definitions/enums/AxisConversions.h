#pragma once

namespace oly::detail
{
	enum class Swizzle
	{
		None = 0,
		YX,
		XZY,
		YXZ,
		YZX,
		ZXY,
		ZYX
	};

	enum class Axis0dConversion
	{
		None = 0,
		To1D,
		To2D,
		To3D
	};

	enum class Axis1dConversion
	{
		None = 0,
		To0D,
		To2D,
		To3D
	};

	enum class Axis2dConversion
	{
		None = 0,
		To0D_X,
		To0D_Y,
		To0D_XY,
		To1D_X,
		To1D_Y,
		To1D_XY,
		To3D_0,
		To3D_1
	};
}
