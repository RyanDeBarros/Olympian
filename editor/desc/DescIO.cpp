#include "DescIO.h"

#include "gui/scopes/DisabledSection.h"
#include "gui/scopes/Subform.h"

#include "definitions/Keys.h"
#include "definitions/enums/Include.h"

#include <imgui.h>

#include <span>

namespace oly::editor
{
	void DescIO::Draw(const char* label, int& data, const int& def, const char** names, size_t count)
	{
		RowInputData(label, data, def, names, count);
	}

	void DescIO::Draw(const char* label, std::string* data, const std::string* def, size_t count)
	{
		if (auto subform = Subform(label))
		{
			for (size_t i = 0; i < count; ++i)
				Draw(std::to_string(i).c_str(), data[i], def[i]);
		}
	}

	void DescIO::Draw(const char* label, bool* data, const bool* def, const char** sublabels, size_t count)
	{
		Draw(label, data, def, sublabels, nullptr, count);
	}

	void DescIO::Draw(const char* label, bool* data, const bool* def, const char** sublabels, const bool* disabled, size_t count)
	{
		gui::IDScope scope(data);
		gui::PropertyGrid::Key::SetLabel(label);

		for (size_t i = 0; i < count; ++i)
		{
			if (data[i] != def[i])
			{
				gui::PropertyGrid::Reset::Button();
				break;
			}
		}

		gui::PropertyGrid::Value::AddComponent(comp::Generic([data, sublabels, disabled, count]() -> DrawResult {
			DrawResult result;
			for (size_t i = 0; i < count; ++i)
			{
				if (auto d = DisabledSection(disabled && disabled[i]))
				{
					result |= gui::InputData<bool>{}(sublabels[i], data[i]);
					result.Query();
				}

				if (i + 1 < count)
					ImGui::SameLine();
			}
			return result;
			}));

		gui::PropertyGrid::SubmitRow();

		if (gui::PropertyGrid::Reset::AnyActivated())
		{
			for (size_t i = 0; i < count; ++i)
				data[i] = def[i];
		}
	}

	void DescIO::Draw(const char* label, Rect& data, const Rect& def)
	{
		gui::IDScope scope(&data);
		gui::PropertyGrid::Key::SetLabel(label);
		if (data != def)
			gui::PropertyGrid::Reset::Button();

		ValueInputData("x1", data.x1);
		ValueInputData("x2", data.x2);
		ValueInputData("y1", data.y1);
		ValueInputData("y2", data.y2);
		
		gui::PropertyGrid::SubmitRow();
		if (gui::PropertyGrid::Reset::AnyActivated())
			data = def;
	}
	
	void DescIO::Draw(const char* label, UVRect& data, const UVRect& def)
	{
		gui::IDScope scope(&data);
		gui::PropertyGrid::Key::SetLabel(label);
		if (data != def)
			gui::PropertyGrid::Reset::Button();

		ValueInputData("x1", data.x1, MakeOpt(0.f), MakeOpt(1.f));
		ValueInputData("x2", data.x2, MakeOpt(0.f), MakeOpt(1.f));
		ValueInputData("y1", data.y1, MakeOpt(0.f), MakeOpt(1.f));
		ValueInputData("y2", data.y2, MakeOpt(0.f), MakeOpt(1.f));

		gui::PropertyGrid::SubmitRow();
		if (gui::PropertyGrid::Reset::AnyActivated())
			data = def;
	}
	
	void DescIO::Draw(const char* label, TopSidePadding& data, const TopSidePadding& def)
	{
		gui::IDScope scope(&data);
		gui::PropertyGrid::Key::SetLabel(label);
		if (data != def)
			gui::PropertyGrid::Reset::Button();

		ValueInputData("left", data.left);
		ValueInputData("right", data.right);
		ValueInputData("top", data.top);

		gui::PropertyGrid::SubmitRow();
		if (gui::PropertyGrid::Reset::AnyActivated())
			data = def;
	}

	template<>
	DrawResult DescIO::DrawCombo(const char* label, detail::Axis0dConversion& data)
	{
		return DrawEnumCombo(label, data, { "None", "To 1D", "To 2D", "To 3D" });
	}

	template<>
	DrawResult DescIO::DrawCombo(const char* label, detail::Axis1dConversion& data)
	{
		return DrawEnumCombo(label, data, { "None", "To 0D", "To 2D", "To 3D" });
	}

	template<>
	DrawResult DescIO::DrawCombo(const char* label, detail::Axis2dConversion& data)
	{
		return DrawEnumCombo(label, data, { "None", "To 0D (X)", "To 0D (Y)", "To 0D (XY)", "To 1D (X)", "To 1D (Y)", "To 1D (XY)", "To 3D (z=0)", "To 3D (z=1)" });
	}

	template<>
	DrawResult DescIO::DrawCombo(const char* label, detail::CommonBufferPreset& data)
	{
		return DrawEnumCombo(label, data, { "Common", "Alphanumeric", "Numeric", "Alphabet", "Alphabet (lowercase)", "Alphabet (uppercase)" });
	}

	template<>
	DrawResult DescIO::DrawCombo(const char* label, detail::GamepadAxis2D& data)
	{
		return DrawEnumCombo(label, data, { "Left XY", "Right XY" });
	}

	template<>
	DrawResult DescIO::DrawCombo(const char* label, detail::PositioningMode& data)
	{
		return DrawEnumCombo(label, data, { "Relative", "Absolute" });
	}

	template<>
	DrawResult DescIO::DrawCombo(const char* label, detail::SignalBindingType& data)
	{
		return DrawEnumCombo(label, data, { "Key", "Mouse Button", "Gamepad Button", "Gamepad Axis 1D", "Gamepad Axis 2D", "Cursor Position", "Scroll" });
	}

	template<>
	DrawResult DescIO::DrawCombo(const char* label, detail::SpritesheetParamType& data)
	{
		return DrawEnumCombo(label, data, { "Index", "Pixel" });
	}

	template<>
	DrawResult DescIO::DrawCombo(const char* label, detail::StorageMode& data)
	{
		return DrawEnumCombo(label, data, { "Discard", "Keep" });
	}

	template<>
	DrawResult DescIO::DrawCombo(const char* label, detail::Swizzle& data)
	{
		return DrawEnumCombo(label, data, { "None", "YX", "XZY", "YXZ", "YZX", "ZXY", "ZYX" });
	}

	template<>
	DrawResult DescIO::DrawCombo(const char* label, detail::SVGMipmapGenerationMode& data)
	{
		return DrawEnumCombo(label, data, { "Auto", "Off", "Manual" });
	}

	template<>
	DrawResult DescIO::DrawCombo(const char* label, detail::TileRotation& data)
	{
		return DrawEnumCombo(label, data, { "None", "90 degrees", "180 degrees", "270 degrees" });
	}
}
