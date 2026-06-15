#include "DescIO.h"

#include "core/editor/ResourceLoader.h"
#include "gui/DisabledSection.h"
#include "gui/DynamicList.h"
#include "gui/ImGuiWrapper.h"
#include "gui/Subform.h"
#include "gui/Toolbar.h"

#include "definitions/Keys.h"
#include "definitions/enums/Include.h"

#include <imgui.h>

namespace oly::editor
{
	void DescIO::PrepareValue(const char* label)
	{
		ImGui::TableNextRow();
		ImGui::TableNextColumn();
		ImGui::Text(label);
		ImGui::TableNextColumn();
	}

	bool DescIO::DrawRevertButton()
	{
		bool dirty = false;
		ImGui::SameLine();
		if (Toolbar::DrawIconButton(IconResource::Revert, "Reset to default", "##Revert"))
			dirty = true;
		return dirty;
	}

	bool DescIO::Draw(const char* label, int& data, const int& def, const char** names, size_t count)
	{
		bool dirty = false;
		PrepareValue(label);
		gui::IDScope scope(&data);
		dirty |= gui::InputData<int>{}("", data, names, count);
		dirty |= CheckRevertButton(data, def);
		return dirty;
	}

	bool DescIO::Draw(const char* label, std::string* data, const std::string* def, size_t count)
	{
		std::unique_ptr<Form> temp_form;
		Form* form = Form::ActiveForm();
		if (!form)
		{
			temp_form = std::make_unique<Form>();
			form = temp_form.get();
		}

		if (auto subform = Subform(*form, label))
		{
			bool dirty = false;
			for (size_t i = 0; i < count; ++i)
				dirty |= Draw(std::to_string(i).c_str(), data[i], def[i]);
			return dirty;
		}
		else
			return false;
	}

	bool DescIO::Draw(const char* label, bool* data, const bool* def, const char** sublabels, size_t count)
	{
		bool dirty = false;
		PrepareValue(label);
		gui::IDScope scope(&data);

		for (size_t i = 0; i < count; ++i)
		{
			dirty |= ImGui::Checkbox(sublabels[i], data + i);
			dirty |= CheckRevertButton(data[i], def[i]);

			if (i + 1 < count)
				ImGui::SameLine();
		}

		return dirty;
	}

	template<>
	bool DescIO::DrawCombo(const char* label, detail::Axis0dConversion& data)
	{
		return DrawEnumCombo(label, data, { "None", "To 1D", "To 2D", "To 3D" });
	}

	template<>
	bool DescIO::DrawCombo(const char* label, detail::Axis1dConversion& data)
	{
		return DrawEnumCombo(label, data, { "None", "To 0D", "To 2D", "To 3D" });
	}

	template<>
	bool DescIO::DrawCombo(const char* label, detail::Axis2dConversion& data)
	{
		return DrawEnumCombo(label, data, { "None", "To 0D (X)", "To 0D (Y)", "To 0D (XY)", "To 1D (X)", "To 1D (Y)", "To 1D (XY)", "To 3D (z=0)", "To 3D (z=1)" });
	}

	template<>
	bool DescIO::DrawCombo(const char* label, detail::CommonBufferPreset& data)
	{
		return DrawEnumCombo(label, data, { "Common", "Alphanumeric", "Numeric", "Alphabet", "Alphabet (lowercase)", "Alphabet (uppercase)" });
	}

	template<>
	bool DescIO::DrawCombo(const char* label, detail::GamepadAxis2D& data)
	{
		return DrawEnumCombo(label, data, { "Left XY", "Right XY" });
	}

	template<>
	bool DescIO::DrawCombo(const char* label, detail::SignalBindingType& data)
	{
		return DrawEnumCombo(label, data, { "Key", "Mouse Button", "Gamepad Button", "Gamepad Axis 1D", "Gamepad Axis 2D", "Cursor Position", "Scroll" });
	}

	template<>
	bool DescIO::DrawCombo(const char* label, detail::SpritesheetParamType& data)
	{
		return DrawEnumCombo(label, data, { "Index", "Pixel" });
	}

	template<>
	bool DescIO::DrawCombo(const char* label, detail::StorageMode& data)
	{
		return DrawEnumCombo(label, data, { "Discard", "Keep" });
	}

	template<>
	bool DescIO::DrawCombo(const char* label, detail::Swizzle& data)
	{
		return DrawEnumCombo(label, data, { "None", "YX", "XZY", "YXZ", "YZX", "ZXY", "ZYX" });
	}

	template<>
	bool DescIO::DrawCombo(const char* label, detail::SVGMipmapGenerationMode& data)
	{
		return DrawEnumCombo(label, data, { "Auto", "Off", "Manual" });
	}

	template<>
	bool DescIO::DrawCombo(const char* label, detail::TileConfiguration& data)
	{
		return DrawEnumCombo(label, data, {
			"Single",
			"End 1",
			"End 2",
			"End 3",
			"End 4",
			"Corner 1",
			"Corner 2",
			"Corner 3",
			"Corner 4",
			"I-Line 1",
			"I-Line 2",
			"T-Bone 1",
			"T-Bone 2",
			"T-Bone 3",
			"T-Bone 4",
			"Middle",
			"Corner (') 1",
			"Corner (') 2",
			"Corner (') 3",
			"Corner (') 4",
			"T-Bone (+) 1",
			"T-Bone (+) 2",
			"T-Bone (+) 3",
			"T-Bone (+) 4",
			"T-Bone (-) 1",
			"T-Bone (-) 2",
			"T-Bone (-) 3",
			"T-Bone (-) 4",
			"T-Bone (') 1",
			"T-Bone (') 2",
			"T-Bone (') 3",
			"T-Bone (') 4",
			"Middle Corner 1",
			"Middle Corner 2",
			"Middle Corner 3",
			"Middle Corner 4",
			"Middle T-Bone 1",
			"Middle T-Bone 2",
			"Middle T-Bone 3",
			"Middle T-Bone 4",
			"Middle Across 1",
			"Middle Across 2",
			"Middle Diagonal 1",
			"Middle Diagonal 2",
			"Middle Diagonal 3",
			"Middle Diagonal 4",
			"Middle (')"
		});
	}

	template<>
	bool DescIO::DrawCombo(const char* label, detail::TileTransformation& data)
	{
		return DrawEnumCombo(label, data, { "None", "Reflect (X)", "Reflect (Y)", "Rotate 90", "Rotate 180", "Rotate 270" });
	}
}
