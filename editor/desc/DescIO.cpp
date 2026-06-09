#include "DescIO.h"

#include "core/ResourceLoader.h"
#include "gui/DisabledSection.h"
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

	bool DescIO::Draw(const char* label, std::vector<std::string>& data, const std::vector<std::string>& def)
	{
		bool dirty = false;
		PrepareValue(label);
		gui::IDScope scope(&data);

		// TODO v8 '+', '-', and 'X' buttons. Revert button next to them corresponding to default size

		for (size_t i = 0; i < data.size(); ++i)
		{
			scope.Push(static_cast<int>(i));
			dirty |= gui::InputText("##Item", data[i]);

			if (i < def.size())
				dirty |= CheckRevertButton(data[i], def[i]);
			else
			{
				static const std::string empty = "";
				dirty |= CheckRevertButton(data[i], empty);
			}
		}

		return dirty;
	}

	template<>
	bool DescIO::Draw(const char* label, detail::Axis0dConversion& data, const detail::Axis0dConversion& def)
	{
		return DrawEnum(label, data, def, { "None", "To 1D", "To 2D", "To 3D" });
	}

	template<>
	bool DescIO::Draw(const char* label, detail::Axis1dConversion& data, const detail::Axis1dConversion& def)
	{
		return DrawEnum(label, data, def, { "None", "To 0D", "To 2D", "To 3D" });
	}

	template<>
	bool DescIO::Draw(const char* label, detail::Axis2dConversion& data, const detail::Axis2dConversion& def)
	{
		return DrawEnum(label, data, def, { "None", "To 0D (X)", "To 0D (Y)", "To 0D (XY)", "To 1D (X)", "To 1D (Y)", "To 1D (XY)", "To 3D (z=0)", "To 3D (z=1)" });
	}

	template<>
	bool DescIO::Draw(const char* label, detail::GamepadAxis2D& data, const detail::GamepadAxis2D& def)
	{
		return DrawEnum(label, data, def, { "Left XY", "Right XY" });
	}

	template<>
	bool DescIO::Draw(const char* label, detail::SignalBindingType& data, const detail::SignalBindingType& def)
	{
		return DrawEnum(label, data, def, { "Key", "Mouse Button", "Gamepad Button", "Gamepad Axis 1D", "Gamepad Axis 2D", "Cursor Position", "Scroll" });
	}

	template<>
	bool DescIO::Draw(const char* label, detail::SpritesheetParamType& data, const detail::SpritesheetParamType& def)
	{
		return DrawEnum(label, data, def, { "Index", "Pixel" });
	}

	template<>
	bool DescIO::Draw(const char* label, detail::StorageMode& data, const detail::StorageMode& def)
	{
		return DrawEnum(label, data, def, { "Discard", "Keep" });
	}

	template<>
	bool DescIO::Draw(const char* label, detail::Swizzle& data, const detail::Swizzle& def)
	{
		return DrawEnum(label, data, def, { "None", "YX", "XZY", "YXZ", "YZX", "ZXY", "ZYX" });
	}

	template<>
	bool DescIO::Draw(const char* label, detail::SVGMipmapGenerationMode& data, const detail::SVGMipmapGenerationMode& def)
	{
		return DrawEnum(label, data, def, { "Auto", "Off", "Manual" });
	}
}
