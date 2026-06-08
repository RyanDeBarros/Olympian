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
	static void PrepareValue(const char* label, const void* data)
	{
		ImGui::TableNextRow();
		ImGui::TableNextColumn();
		ImGui::Text(label);
		ImGui::TableNextColumn();
		ImGui::PushID(data);
	}

	static bool DrawRevertButton(const void* data)
	{
		bool dirty = false;
		ImGui::SameLine();
		if (Toolbar::DrawIconButton(IconResource::Revert, "Revert", data))
			dirty = true;
		return dirty;
	}

	template<typename T>
	static bool FinishValue(bool dirty, T& desc, const T* disk)
	{
		if (disk && desc != *disk && DrawRevertButton(disk))
		{
			desc = *disk;
			dirty = true;
		}
		ImGui::PopID();
		return dirty;
	}

	bool DescIO::Draw(const char* label, bool& data, const bool* disk)
	{
		bool dirty = false;
		PrepareValue(label, &data);
		if (ImGui::Checkbox("", &data))
			dirty = true;
		return FinishValue(dirty, data, disk);
	}

	template<typename T>
	static bool Clamp(T& data, const T og, OptionalPrimitive<T> min, OptionalPrimitive<T> max)
	{
		if (max.has_value)
			data = std::min(data, max.value);
		if (min.has_value)
			data = std::max(data, min.value);
		return data != og;
	}

	template<typename T>
	static float GetItemComfortableWidth(OptionalPrimitive<T> num)
	{
		if (num.has_value)
			return ImGui::CalcTextSize(std::to_string(num.value).c_str()).x;
		else
			return ImGui::CalcTextSize(std::to_string(-FLT_MAX).c_str()).x;
	}

	template<typename T>
	static void SetItemComfortableWidth(OptionalPrimitive<T> min, OptionalPrimitive<T> max)
	{
		const float item_width = std::max(GetItemComfortableWidth(min), GetItemComfortableWidth(max));
		ImGui::SetNextItemWidth(item_width);
	}

	bool DescIO::Draw(const char* label, int& data, const int* disk, OptionalInt min, OptionalInt max)
	{
		bool dirty = false;
		PrepareValue(label, &data);
		SetItemComfortableWidth(min, max);
		const auto og = data;
		if (ImGui::InputInt("", &data))
			dirty |= Clamp(data, og, min, max);
		return FinishValue(dirty, data, disk);
	}

	bool DescIO::Draw(const char* label, float& data, const float* disk, OptionalFloat min, OptionalFloat max)
	{
		bool dirty = false;
		PrepareValue(label, &data);
		SetItemComfortableWidth(min, max);
		const auto og = data;
		if (ImGui::InputFloat("", &data))
			dirty |= Clamp(data, og, min, max);
		return FinishValue(dirty, data, disk);
	}

	bool DescIO::Draw(const char* label, double& data, const double* disk, OptionalDouble min, OptionalDouble max)
	{
		bool dirty = false;
		PrepareValue(label, &data);
		SetItemComfortableWidth(min, max);
		const auto og = data;
		if (ImGui::InputDouble("", &data))
			dirty |= Clamp(data, og, min, max);
		return FinishValue(dirty, data, disk);
	}

	bool DescIO::Draw(const char* label, OptionalInt& data, const OptionalInt* disk, OptionalInt min, OptionalInt max)
	{
		bool dirty = false;
		PrepareValue(label, &data);
		dirty |= ImGui::Checkbox("##Enable", &data.has_value);
		if (auto disabled = DisabledSection(!data.has_value))
		{
			ImGui::SameLine();
			SetItemComfortableWidth(min, max);
			const int og = data.value;
			if (ImGui::InputInt("##Value", &data.value))
				dirty |= Clamp(data.value, og, min, max);
		}
		return FinishValue(dirty, data, disk);
	}

	bool DescIO::Draw(const char* label, OptionalFloat& data, const OptionalFloat* disk, OptionalFloat min, OptionalFloat max)
	{
		bool dirty = false;
		PrepareValue(label, &data);
		dirty |= ImGui::Checkbox("##Enable", &data.has_value);
		if (auto disabled = DisabledSection(!data.has_value))
		{
			ImGui::SameLine();
			SetItemComfortableWidth(min, max);
			const float og = data.value;
			if (ImGui::InputFloat("##Value", &data.value))
				dirty |= Clamp(data.value, og, min, max);
		}
		return FinishValue(dirty, data, disk);
	}

	bool DescIO::Draw(const char* label, int& data, const int* disk, const char** names, size_t count)
	{
		bool dirty = false;
		PrepareValue(label, &data);
		if (ImGui::Combo("", &data, names, count))
			dirty = true;
		return FinishValue(dirty, data, disk);
	}

	template<typename E, size_t N>
	static bool DrawEnum(const char* label, E& data, const E* disk, const char* const (&values)[N])
	{
		bool dirty = false;
		int index = static_cast<int>(data);
		PrepareValue(label, &data);
		if (ImGui::Combo("", &index, values, N))
			dirty = true;
		data = static_cast<E>(index);
		return FinishValue(dirty, data, disk);
	}

	bool DescIO::Draw(const char* label, std::string& data, const std::string* disk)
	{
		bool dirty = false;
		PrepareValue(label, &data);
		if (gui::InputText("", data))
			dirty = true;
		return FinishValue(dirty, data, disk);
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

	bool DescIO::DrawColor(const char* label, glm::vec4& data, const glm::vec4* disk)
	{
		bool dirty = false;
		PrepareValue(label, &data);
		if (ImGui::ColorEdit4("", glm::value_ptr(data)))
			dirty = true;
		return FinishValue(dirty, data, disk);
	}

	template<>
	bool DescIO::Draw(const char* label, detail::StorageMode& data, const detail::StorageMode* disk)
	{
		return DrawEnum(label, data, disk, { "Discard", "Keep" });
	}

	template<>
	bool DescIO::Draw(const char* label, detail::SVGMipmapGenerationMode& data, const detail::SVGMipmapGenerationMode* disk)
	{
		return DrawEnum(label, data, disk, { "Auto", "Off", "Manual" });
	}

	template<>
	bool DescIO::Draw(const char* label, detail::SpritesheetParamType& data, const detail::SpritesheetParamType* disk)
	{
		return DrawEnum(label, data, disk, { "Index", "Pixel" });
	}
}
