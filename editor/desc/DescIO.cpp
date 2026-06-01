#include "DescIO.h"

#include "definitions/Keys.h"

#include <imgui.h>

namespace oly::editor
{
	static void PrepareValue(const char* label, void* data)
	{
		ImGui::TableNextRow();
		ImGui::TableNextColumn();
		ImGui::Text(label);
		ImGui::TableNextColumn();
		ImGui::PushID(data);
	}

	template<typename T>
	static bool DrawRevertButton(T& desc, const T& disk)
	{
		bool dirty = false;
		ImGui::SameLine();
		ImGui::PushID(&disk);
		if (ImGui::ArrowButton("", ImGuiDir_Left))
		{
			desc = disk;
			dirty = true;
		}
		ImGui::PopID();
		return dirty;
	}

	template<typename T>
	static bool FinishValue(bool dirty, T& desc, const T* disk)
	{
		if (disk && desc != *disk)
			dirty |= DrawRevertButton(desc, *disk);
		ImGui::PopID();
		return dirty;
	}

	bool DescIO::BeginForm(void* id)
	{
		ImGui::PushID(id);
		if (ImGui::BeginTable("", 2, ImGuiTableFlags_SizingFixedFit))
		{
			ImGui::TableSetupColumn("");
			ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthStretch);
			return true;
		}
		else
		{
			ImGui::PopID();
			return false;
		}
	}

	void DescIO::EndForm()
	{
		ImGui::EndTable();
		ImGui::PopID();
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
}
