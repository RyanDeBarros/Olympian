#include "DescIO.h"

#include "definitions/Keys.h"

#include <imgui.h>

namespace oly::editor
{
	// TODO v7 on hover tooltip
	// TODO v7 revert buttons

	static void PrepareValue(const char* label, void* data)
	{
		ImGui::TableNextRow();
		ImGui::TableNextColumn();
		ImGui::Text(label);
		ImGui::TableNextColumn();
		ImGui::PushID(data);
	}

	static void FinishValue()
	{
		ImGui::PopID();
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

	bool DescIO::Draw(const char* label, bool& data)
	{
		bool dirty = false;
		PrepareValue(label, &data);
		if (ImGui::Checkbox("", &data))
			dirty = true;
		FinishValue();
		return dirty;
	}

	bool DescIO::Draw(const char* label, int& data, std::optional<int> min, std::optional<int> max)
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
		FinishValue();
		return dirty;
	}

	bool DescIO::Draw(const char* label, float& data, std::optional<float> min, std::optional<float> max)
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
		FinishValue();
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

		PrepareValue(label, &data);
		if (ImGui::Combo("", &index, names, count))
			dirty = true;
		FinishValue();

		data = values[index];
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
		PrepareValue(label, &data);
		if (ImGui::Combo("", &index, values, IM_ARRAYSIZE(values)))
			dirty = true;
		FinishValue();
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
		PrepareValue(label, &data);
		if (ImGui::Combo("", &index, values, IM_ARRAYSIZE(values)))
			dirty = true;
		FinishValue();
		data = static_cast<detail::SVGMipmapGenerationMode>(index);
		return dirty;
	}

	void DescIO::Load(TOMLNode node, bool& data, detail::Key key, bool def)
	{
		data = node[detail::decode_key(key)].value_or(def);
	}

	void DescIO::Load(TOMLNode node, int& data, detail::Key key, int def)
	{
		data = node[detail::decode_key(key)].value_or<int64_t>(def);
	}

	void DescIO::Load(TOMLNode node, float& data, detail::Key key, float def)
	{
		data = node[detail::decode_key(key)].value_or<double>(def);
	}

	void DescIO::Load(TOMLNode node, GLenum& data, detail::Key key, GLenum def)
	{
		data = node[detail::decode_key(key)].value_or<int64_t>(def);
	}

	void DescIO::Dump(toml::table& table, detail::Key key, bool data)
	{
		table.insert_or_assign(detail::decode_key(key), data);
	}
	
	void DescIO::Dump(toml::table& table, detail::Key key, int data)
	{
		table.insert_or_assign(detail::decode_key(key), static_cast<int64_t>(data));
	}
	
	void DescIO::Dump(toml::table& table, detail::Key key, float data)
	{
		table.insert_or_assign(detail::decode_key(key), data);
	}
	
	void DescIO::Dump(toml::table& table, detail::Key key, GLenum data)
	{
		table.insert_or_assign(detail::decode_key(key), static_cast<int64_t>(data));
	}
}
