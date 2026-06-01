#include "DescIO.h"

#include <imgui.h>

namespace oly::editor
{
	// TODO v7 on hover tooltip

	bool DescIO::Draw(const char* label, bool& data)
	{
		bool dirty = false;
		ImGui::PushID(&data);
		if (ImGui::Checkbox(label, &data))
			dirty = true;
		ImGui::PopID();
		return dirty;
	}

	bool DescIO::Draw(const char* label, int& data, std::optional<int> min, std::optional<int> max)
	{
		bool dirty = false;
		const int og = data;
		ImGui::PushID(&data);
		if (ImGui::InputInt(label, &data))
		{
			if (max)
				data = std::min(data, *max);
			if (min)
				data = std::max(data, *min);
			dirty = data != og;
		}
		ImGui::PopID();
		return dirty;
	}

	bool DescIO::Draw(const char* label, detail::StorageMode& data)
	{
		bool dirty = false;
		static const char* values[] = {
			"Discard",
			"Keep"
		};
		int index = static_cast<int>(data);
		ImGui::PushID(&data);
		if (ImGui::Combo(label, &index, values, IM_ARRAYSIZE(values)))
			dirty = true;
		ImGui::PopID();
		data = static_cast<detail::StorageMode>(index);
		return dirty;
	}

	bool DescIO::Draw(const char* label, detail::SVGMipmapGenerationMode& data)
	{
		bool dirty = false;
		static const char* values[] = {
			"Auto",
			"Off",
			"Manual"
		};
		int index = static_cast<int>(data);
		ImGui::PushID(&data);
		if (ImGui::Combo(label, &index, values, IM_ARRAYSIZE(values)))
			dirty = true;
		ImGui::PopID();
		data = static_cast<detail::SVGMipmapGenerationMode>(index);
		return dirty;
	}

	bool DescIO::Draw(const char* label, GLenum& data, const GLenum* values, const char** names, size_t count)
	{
		bool dirty = false;
		
		int index = 0;
		for (size_t i = 0; i < count; ++i)
		{
			if (values[i] == data)
			{
				index = i;
				break;
			}
		}

		ImGui::PushID(&data);
		if (ImGui::Combo(label, &index, names, count))
			dirty = true;
		ImGui::PopID();

		data = values[index];
		return dirty;
	}
}
