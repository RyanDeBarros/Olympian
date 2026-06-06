#include "DescIO.h"

#include "graphics/Toolbar.h"
#include "core/ResourceLoader.h"

#include "definitions/Keys.h"
#include "definitions/enums/Include.h"

#include <imgui.h>

#include <string>

namespace oly::editor
{
	// TODO v8 OOP model for PrepareValue/FinishValue/DrawRevertButton
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

	bool DescIO::Draw(const char* label, int& data, const int* disk, OptionalInt min, OptionalInt max)
	{
		bool dirty = false;
		PrepareValue(label, &data);
		const int og = data;
		if (ImGui::InputInt("", &data))
		{
			if (max.has_value)
				data = std::min(data, max.value);
			if (min.has_value)
				data = std::max(data, min.value);
			dirty = data != og;
		}
		return FinishValue(dirty, data, disk);
	}

	bool DescIO::Draw(const char* label, float& data, const float* disk, OptionalFloat min, OptionalFloat max)
	{
		bool dirty = false;
		PrepareValue(label, &data);
		const float og = data;
		if (ImGui::InputFloat("", &data))
		{
			if (max.has_value)
				data = std::min(data, max.value);
			if (min.has_value)
				data = std::max(data, min.value);
			dirty = data != og;
		}
		return FinishValue(dirty, data, disk);
	}

	bool DescIO::Draw(const char* label, OptionalInt& data, const OptionalInt* disk, OptionalInt min, OptionalInt max)
	{
		bool dirty = false;
		PrepareValue(label, &data);
		dirty |= ImGui::Checkbox("", &data.has_value);
		ImGui::BeginDisabled(!data.has_value);
		const int og = data.value;
		ImGui::SameLine();
		if (ImGui::InputInt("", &data.value))
		{
			if (max.has_value)
				data.value = std::min(data.value, max.value);
			if (min.has_value)
				data.value = std::max(data.value, min.value);
			dirty |= data.value != og;
		}
		ImGui::EndDisabled();
		return FinishValue(dirty, data, disk);
	}

	bool DescIO::Draw(const char* label, OptionalFloat& data, const OptionalFloat* disk, OptionalFloat min, OptionalFloat max)
	{
		bool dirty = false;
		PrepareValue(label, &data);
		dirty |= ImGui::Checkbox("", &data.has_value);
		ImGui::BeginDisabled(!data.has_value);
		const float og = data.value;
		ImGui::SameLine();
		if (ImGui::InputFloat("", &data.value))
		{
			if (max.has_value)
				data.value = std::min(data.value, max.value);
			if (min.has_value)
				data.value = std::max(data.value, min.value);
			dirty |= data.value != og;
		}
		ImGui::EndDisabled();
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

	template<>
	bool DescIO::Draw(const char* label, detail::StorageMode& data, const detail::StorageMode* disk)
	{
		bool dirty = false;
		static const char* values[] = {
			"Discard",
			"Keep"
		};
		int index = static_cast<int>(data);
		PrepareValue(label, &data);
		if (ImGui::Combo("", &index, values, IM_ARRAYSIZE(values)))
			dirty = true;
		data = static_cast<detail::StorageMode>(index);
		return FinishValue(dirty, data, disk);
	}

	template<>
	bool DescIO::Draw(const char* label, detail::SVGMipmapGenerationMode& data, const detail::SVGMipmapGenerationMode* disk)
	{
		bool dirty = false;
		static const char* values[] = {
			"Auto",
			"Off",
			"Manual"
		};
		int index = static_cast<int>(data);
		PrepareValue(label, &data);
		if (ImGui::Combo("", &index, values, IM_ARRAYSIZE(values)))
			dirty = true;
		data = static_cast<detail::SVGMipmapGenerationMode>(index);
		return FinishValue(dirty, data, disk);
	}

	template<>
	bool DescIO::Draw(const char* label, detail::SpritesheetParamType& data, const detail::SpritesheetParamType* disk)
	{
		bool dirty = false;
		static const char* values[] = {
			"Index",
			"Pixel"
		};
		int index = static_cast<int>(data);
		PrepareValue(label, &data);
		if (ImGui::Combo("", &index, values, IM_ARRAYSIZE(values)))
			dirty = true;
		data = static_cast<detail::SpritesheetParamType>(index);
		return FinishValue(dirty, data, disk);
	}

	const char* DescIO::StringVectorComboGetter(void* data, int idx)
	{
		auto& items = *static_cast<std::vector<std::string>*>(data);
		if (idx < 0 || idx >= items.size())
			return nullptr;
		else
			return items[idx].c_str();
	}
}
