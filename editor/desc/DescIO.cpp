#include "DescIO.h"

#include "graphics/Toolbar.h"
#include "core/ResourceLoader.h"

#include "definitions/Keys.h"
#include "definitions/enums/Include.h"

#include <imgui.h>

#include <string>

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

	static bool DrawRevertButtonImpl(const void* data)
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
		if (disk && desc != *disk && DrawRevertButtonImpl(disk))
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

	bool DescIO::Draw(const char* label, int& data, const int* disk, std::optional<int> min, std::optional<int> max)
	{
		bool dirty = false;
		const int og = data;
		PrepareValue(label, &data);
		if (ImGui::InputInt("", &data))
		{
			if (max)
				data = std::min(data, *max);
			if (min)
				data = std::max(data, *min);
			dirty = data != og;
		}
		return FinishValue(dirty, data, disk);
	}

	bool DescIO::Draw(const char* label, float& data, const float* disk, std::optional<float> min, std::optional<float> max)
	{
		bool dirty = false;
		const float og = data;
		PrepareValue(label, &data);
		if (ImGui::InputFloat("", &data))
		{
			if (max)
				data = std::min(data, *max);
			if (min)
				data = std::max(data, *min);
			dirty = data != og;
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

	const char* DescIO::StringVectorComboGetter(void* data, int idx)
	{
		auto& items = *static_cast<std::vector<std::string>*>(data);
		if (idx < 0 || idx >= items.size())
			return nullptr;
		else
			return items[idx].c_str();
	}
}
