#include "Fields.h"

#include "core/MemoryUnit.h"

#include "definitions/enums/Include.h"

namespace imtk::field
{
	template<>
	imtk::label_span_registry::handle enum_fld<oly::editor::MemoryUnit>::combo_names()
	{
		return imtk::label_span_registry::intern({ "B", "KB", "KiB", "MB", "MiB", "GB", "GiB" });
	}

	template<>
	imtk::label_span_registry::handle enum_fld<oly::detail::Axis0dConversion>::combo_names()
	{
		return imtk::label_span_registry::intern({ "None", "To 1D", "To 2D", "To 3D" });
	}

	template<>
	imtk::label_span_registry::handle enum_fld<oly::detail::Axis1dConversion>::combo_names()
	{
		return imtk::label_span_registry::intern({ "None", "To 0D", "To 2D", "To 3D" });
	}

	template<>
	imtk::label_span_registry::handle enum_fld<oly::detail::Axis2dConversion>::combo_names()
	{
		return imtk::label_span_registry::intern({ "None", "To 0D (X)", "To 0D (Y)", "To 0D (XY)", "To 1D (X)", "To 1D (Y)", "To 1D (XY)", "To 3D (z=0)", "To 3D (z=1)" });
	}

	template<>
	imtk::label_span_registry::handle enum_fld<oly::detail::CommonBufferPreset>::combo_names()
	{
		return imtk::label_span_registry::intern({ "Common", "Alphanumeric", "Numeric", "Alphabet", "Alphabet (lowercase)", "Alphabet (uppercase)" });
	}

	template<>
	imtk::label_span_registry::handle enum_fld<oly::detail::GamepadAxis2D>::combo_names()
	{
		return imtk::label_span_registry::intern({ "Left XY", "Right XY" });
	}

	template<>
	imtk::label_span_registry::handle enum_fld<oly::detail::PositioningMode>::combo_names()
	{
		return imtk::label_span_registry::intern({ "Relative", "Absolute" });
	}

	template<>
	imtk::label_span_registry::handle enum_fld<oly::detail::SignalBindingType>::combo_names()
	{
		return imtk::label_span_registry::intern({ "Key", "Mouse Button", "Gamepad Button", "Gamepad Axis 1D", "Gamepad Axis 2D", "Cursor Position", "Scroll" });
	}

	template<>
	imtk::label_span_registry::handle enum_fld<oly::detail::SpritesheetParamType>::combo_names()
	{
		return imtk::label_span_registry::intern({ "Index", "Pixel" });
	}

	template<>
	imtk::label_span_registry::handle enum_fld<oly::detail::StorageMode>::combo_names()
	{
		return imtk::label_span_registry::intern({ "Discard", "Keep" });
	}

	template<>
	imtk::label_span_registry::handle enum_fld<oly::detail::Swizzle>::combo_names()
	{
		return imtk::label_span_registry::intern({ "None", "YX", "XZY", "YXZ", "YZX", "ZXY", "ZYX" });
	}

	template<>
	imtk::label_span_registry::handle enum_fld<oly::detail::SVGMipmapGenerationMode>::combo_names()
	{
		return imtk::label_span_registry::intern({ "Auto", "Off", "Manual" });
	}

	template<>
	imtk::label_span_registry::handle enum_fld<oly::detail::TileRotation>::combo_names()
	{
		return imtk::label_span_registry::intern({ "None", "90 degrees", "180 degrees", "270 degrees" });
	}
}
