#include "ImGuiWrapper.h"

#include "gui/GUIState.h"

#include <imgui_internal.h>

namespace oly::editor::gui
{
	void VerticalSeparator()
	{
		ImGui::SameLine();
		ImGui::SeparatorEx(ImGuiSeparatorFlags_Vertical);
		ImGui::SameLine();
	}

	static const char* StringVectorComboGetter(void* data, int idx)
	{
		const std::vector<std::string>& items = *static_cast<const std::vector<std::string>*>(data);
		if (idx < 0 || idx >= items.size())
			return nullptr;
		else
			return items[idx].c_str();
	}

	DrawResult Combo(const char* label, int& current_item, const std::vector<std::string>& items)
	{
		auto styles = ApplyStyles(GUIState::input_data_styles);

		float width = 0.0f;
		for (int i = 0; i < items.size(); i++)
			width = std::max(width, ImGui::CalcTextSize(items[i].c_str()).x);
		width += 2 * ImGui::GetFrameHeight(); // roughly covers dropdown arrow + padding
		ImGui::SetNextItemWidth(width);

		return ImGui::Combo(label, &current_item, &StringVectorComboGetter, const_cast<std::vector<std::string>*>(&items), static_cast<int>(items.size()));
	}

	DrawResult InputText(const char* label, std::string& string, size_t max_size, ImGuiInputTextFlags flags, ImGuiInputTextCallback callback, void* user_data)
	{
		std::string buf;
		buf.resize(max_size);
		string.copy(buf.data(), buf.size());
		auto styles = ApplyStyles(GUIState::input_data_styles);
		DrawResult result = ImGui::InputText(label, buf.data(), buf.size(), flags, callback, user_data);
		string = std::move(buf);
		size_t n = string.find('\0');
		if (n != std::string::npos)
			string.resize(n);
		return result.Query();
	}

	DrawResult InputData<bool>::operator()(const char* label, bool& data) const
	{
		auto styles = ApplyStyles(GUIState::input_data_styles);
		DrawResult result = ImGui::Checkbox(label, &data);
		return result.Query();
	}

	DrawResult InputData<int>::operator()(const char* label, int& data) const
	{
		auto styles = ApplyStyles(GUIState::input_data_styles);
		DrawResult result = ImGui::InputInt(label, &data);
		return result.Query();
	}

	DrawResult InputData<int>::operator()(const char* label, int& data, OptionalPrimitive<int> min, OptionalPrimitive<int> max) const
	{
		return InputClampedData(label, data, min, max);
	}

	DrawResult InputData<int>::operator()(const char* label, int& data, const char** names, size_t count)
	{
		auto styles = ApplyStyles(GUIState::input_data_styles);
		DrawResult result = ImGui::Combo(label, &data, names, count);
		return result.Query();
	}

	DrawResult InputData<float>::operator()(const char* label, float& data) const
	{
		auto styles = ApplyStyles(GUIState::input_data_styles);
		DrawResult result = ImGui::InputFloat(label, &data);
		return result.Query();
	}

	DrawResult InputData<float>::operator()(const char* label, float& data, OptionalPrimitive<float> min, OptionalPrimitive<float> max) const
	{
		return InputClampedData(label, data, min, max);
	}

	DrawResult InputData<double>::operator()(const char* label, double& data) const
	{
		auto styles = ApplyStyles(GUIState::input_data_styles);
		DrawResult result = ImGui::InputDouble(label, &data);
		return result.Query();
	}

	DrawResult InputData<double>::operator()(const char* label, double& data, OptionalPrimitive<double> min, OptionalPrimitive<double> max) const
	{
		return InputClampedData(label, data, min, max);
	}

	DrawResult InputData<glm::vec2>::operator()(const char* label, glm::vec2& data) const
	{
		auto styles = ApplyStyles(GUIState::input_data_styles);
		DrawResult result = ImGui::InputFloat2(label, glm::value_ptr(data));
		return result.Query();
	}

	DrawResult InputData<glm::vec2>::operator()(const char* label, glm::vec2& data, OptionalPrimitive<float> min, OptionalPrimitive<float> max) const
	{
		return InputClampedData(label, data, min, max);
	}

	DrawResult InputData<glm::vec3>::operator()(const char* label, glm::vec3& data) const
	{
		auto styles = ApplyStyles(GUIState::input_data_styles);
		DrawResult result = ImGui::InputFloat3(label, glm::value_ptr(data));
		return result.Query();
	}

	DrawResult InputData<glm::vec3>::operator()(const char* label, glm::vec3& data, OptionalPrimitive<float> min, OptionalPrimitive<float> max) const
	{
		return InputClampedData(label, data, min, max);
	}

	DrawResult InputData<glm::vec4>::operator()(const char* label, glm::vec4& data) const
	{
		auto styles = ApplyStyles(GUIState::input_data_styles);
		DrawResult result = ImGui::InputFloat4(label, glm::value_ptr(data));
		return result.Query();
	}

	DrawResult InputData<glm::vec4>::operator()(const char* label, glm::vec4& data, OptionalPrimitive<float> min, OptionalPrimitive<float> max) const
	{
		return InputClampedData(label, data, min, max);
	}

	DrawResult InputData<std::string>::operator()(const char* label, std::string& data) const
	{
		return InputText(label, data);
	}

	DrawResult InputData<Color>::operator()(const char* label, Color& data) const
	{
		auto styles = ApplyStyles(GUIState::input_data_styles);
		DrawResult result = ImGui::ColorEdit4(label, data.ValuePtr());
		return result.Query();
	}

	// TODO v9.1 use inner tables more often for layout structuring: define utility to defer calls to input-data so that the number of columns can be dynamically computed + SameLine() automatically called for multiple elements in a column.
	DrawResult InputData<Rect>::operator()(const char* label, Rect& data) const
	{
		const bool header = label && label[0] != '\0';
		DrawResult result;
		if (ImGui::BeginTable("##UVRect", header ? 5 : 4))
		{
			if (header)
			{
				ImGui::TableNextColumn();
				ImGui::Text(label);
				result.Query();
			}

			ImGui::TableNextColumn();
			result |= InputClampedData("x1", data.x1, MakeOpt(0.f), MakeOpt(1.f));
			result.Query();

			ImGui::TableNextColumn();
			result |= InputClampedData("x2", data.x2, MakeOpt(0.f), MakeOpt(1.f));
			result.Query();

			ImGui::TableNextColumn();
			result |= InputClampedData("y1", data.y1, MakeOpt(0.f), MakeOpt(1.f));
			result.Query();

			ImGui::TableNextColumn();
			result |= InputClampedData("y2", data.y2, MakeOpt(0.f), MakeOpt(1.f));
			result.Query();

			ImGui::EndTable();
		}
		return result;
	}

	DrawResult InputData<TopSidePadding>::operator()(const char* label, TopSidePadding& data) const
	{
		const bool header = label && label[0] != '\0';
		DrawResult result;
		if (ImGui::BeginTable("##TopSidePadding", header ? 4 : 3))
		{
			if (header)
			{
				ImGui::TableNextColumn();
				ImGui::Text(label);
				result.Query();
			}

			ImGui::TableNextColumn();
			result |= InputClampedData("Left", data.left, MakeOpt(0.f), MakeOpt(1.f));
			result.Query();

			ImGui::TableNextColumn();
			result |= InputClampedData("Right", data.right, MakeOpt(0.f), MakeOpt(1.f));
			result.Query();

			ImGui::TableNextColumn();
			result |= InputClampedData("Top", data.top, MakeOpt(0.f), MakeOpt(1.f));
			result.Query();

			ImGui::EndTable();
		}
		return result;
	}

	DrawResult InputData<unsigned int>::operator()(unsigned int& data, const unsigned int* values, const char** names, const size_t count)
	{
		return (*this)(data, values, names, nullptr, count);
	}

	DrawResult InputData<unsigned int>::operator()(unsigned int& data, const unsigned int* values, const char** names, const bool* disabled, const size_t count)
	{
		DrawResult result;
		for (size_t i = 0; i < count; ++i)
		{
			bool flag = data & values[i];

			if (auto d = DisabledSection(disabled && disabled[i]))
				result |= InputData<bool>{}(names[i], flag);

			if (flag)
				data |= values[i];
			else
				data &= ~values[i];

			if (i + 1 < count)
				ImGui::SameLine();
		}
		return result;
	}
}
