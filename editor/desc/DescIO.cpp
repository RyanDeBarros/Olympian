#include "DescIO.h"

#include "core/ResourceLoader.h"
#include "gui/DisabledSection.h"
#include "gui/ImGuiWrapper.h"
#include "gui/Subform.h"
#include "gui/Toolbar.h"

#include "definitions/Keys.h"
#include "definitions/enums/Include.h"

#include <imgui.h>

// TODO v8 revert button should revert to default instead of disk. Don't pass disk in any of these. Also, rename 'revert' in asset menu to 'reload'. No need for disk* pointers then or even Reset/Isolate!

namespace oly::editor
{
	void DescIO::PrepareValue(const char* label)
	{
		ImGui::TableNextRow();
		ImGui::TableNextColumn();
		ImGui::Text(label);
		ImGui::TableNextColumn();
	}

	bool DescIO::DrawRevertButton(const void* ptr_id)
	{
		bool dirty = false;
		ImGui::SameLine();
		if (Toolbar::DrawIconButton(IconResource::Revert, "Revert", ptr_id))
			dirty = true;
		return dirty;
	}

	bool DescIO::Draw(const char* label, int& data, const int* disk, const char** names, size_t count)
	{
		bool dirty = false;
		PrepareValue(label);
		gui::IDScope scope(&data);
		dirty |= gui::InputData<int>{}("", data, names, count);
		dirty |= CheckRevertButton(data, disk);
		return dirty;
	}

	bool DescIO::Draw(const char* label, std::string* data, const std::string* disk, size_t count)
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
				dirty |= Draw(std::to_string(i).c_str(), data[i], disk ? &disk[i] : nullptr);
			return dirty;
		}
		else
			return false;
	}

	bool DescIO::Draw(const char* label, bool* data, const bool* disk, const char** sublabels, size_t count)
	{
		bool dirty = false;
		PrepareValue(label);
		gui::IDScope scope(&data);

		for (size_t i = 0; i < count; ++i)
		{
			dirty |= ImGui::Checkbox(sublabels[i], data + i);

			if (disk && data[i] != disk[i] && DrawRevertButton(disk + i))
			{
				data[i] = disk[i];
				dirty = true;
			}

			if (i + 1 < count)
				ImGui::SameLine();
		}

		return dirty;
	}

	template<>
	bool DescIO::Draw(const char* label, detail::Axis0dConversion& data, const detail::Axis0dConversion* disk)
	{
		return DrawEnum(label, data, disk, { "None", "To 1D", "To 2D", "To 3D" });
	}

	template<>
	bool DescIO::Draw(const char* label, detail::Axis1dConversion& data, const detail::Axis1dConversion* disk)
	{
		return DrawEnum(label, data, disk, { "None", "To 0D", "To 2D", "To 3D" });
	}

	template<>
	bool DescIO::Draw(const char* label, detail::Axis2dConversion& data, const detail::Axis2dConversion* disk)
	{
		return DrawEnum(label, data, disk, { "None", "To 0D (X)", "To 0D (Y)", "To 0D (XY)", "To 1D (X)", "To 1D (Y)", "To 1D (XY)", "To 3D (z=0)", "To 3D (z=1)" });
	}

	template<>
	bool DescIO::Draw(const char* label, detail::GamepadAxis2D& data, const detail::GamepadAxis2D* disk)
	{
		return DrawEnum(label, data, disk, { "Left XY", "Right XY" });
	}

	template<>
	bool DescIO::Draw(const char* label, detail::SignalBindingType& data, const detail::SignalBindingType* disk)
	{
		return DrawEnum(label, data, disk, { "Key", "Mouse Button", "Gamepad Button", "Gamepad Axis 1D", "Gamepad Axis 2D", "Cursor Position", "Scroll" });
	}

	template<>
	bool DescIO::Draw(const char* label, detail::SpritesheetParamType& data, const detail::SpritesheetParamType* disk)
	{
		return DrawEnum(label, data, disk, { "Index", "Pixel" });
	}

	template<>
	bool DescIO::Draw(const char* label, detail::StorageMode& data, const detail::StorageMode* disk)
	{
		return DrawEnum(label, data, disk, { "Discard", "Keep" });
	}

	template<>
	bool DescIO::Draw(const char* label, detail::Swizzle& data, const detail::Swizzle* disk)
	{
		return DrawEnum(label, data, disk, { "None", "YX", "XZY", "YXZ", "YZX", "ZXY", "ZYX" });
	}

	template<>
	bool DescIO::Draw(const char* label, detail::SVGMipmapGenerationMode& data, const detail::SVGMipmapGenerationMode* disk)
	{
		return DrawEnum(label, data, disk, { "Auto", "Off", "Manual" });
	}
}
