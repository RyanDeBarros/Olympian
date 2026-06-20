#include "DescIO.h"

#include "core/editor/ResourceLoader.h"
#include "gui/scopes/DisabledSection.h"
#include "gui/scopes/Subform.h"
#include "gui/graphics/Toolbar.h"
#include "gui/DynamicList.h"
#include "gui/ImGuiWrapper.h"

#include "definitions/Keys.h"
#include "definitions/enums/Include.h"

#include <imgui.h>

namespace oly::editor
{
	void DescIO::PrepareValue(const char* label)
	{
		ImGui::TableNextRow();
		ImGui::TableSetColumnIndex(0);

		// TODO v9.1 draw value cell first, so height can be determined and the key cell aligned to middle vertically
		ImGui::TextUnformatted(label);

		ImGui::TableSetColumnIndex(1);
	}

	bool DescIO::DrawRevertButton()
	{
		bool dirty = false;
		ImGui::TableSetColumnIndex(2);
		if (Toolbar::DrawIconButton(IconResource::Revert, "Reset to default", "##Revert"))
			dirty = true;
		return dirty;
	}

	DrawResult DescIO::Draw(const char* label, int& data, const int& def, const char** names, size_t count)
	{
		DrawResult result;
		PrepareValue(label);
		gui::IDScope scope(&data);
		result |= gui::InputData<int>{}("", data, names, count);
		result |= CheckRevertButton(data, def);
		return result;
	}

	DrawResult DescIO::Draw(const char* label, std::string* data, const std::string* def, size_t count)
	{
		if (auto subform = Subform(label))
		{
			DrawResult result;
			for (size_t i = 0; i < count; ++i)
				result |= Draw(std::to_string(i).c_str(), data[i], def[i]);
			return result;
		}
		else
			return false;
	}

	DrawResult DescIO::Draw(const char* label, bool* data, const bool* def, const char** sublabels, size_t count)
	{
		DrawResult result;
		PrepareValue(label);
		gui::IDScope scope(&data);

		for (size_t i = 0; i < count; ++i)
		{
			result |= gui::InputData<bool>{}(sublabels[i], data[i]);
			result.Query();
			result |= CheckRevertButton(data[i], def[i]);

			if (i + 1 < count)
				ImGui::SameLine();
		}

		return result;
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
