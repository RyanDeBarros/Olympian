#include "ImGuiWrapper.h"

#include "gui/GUIState.h"
#include "gui/InlineWidget.h"
#include "gui/WidgetComponentCommon.h"

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

		return DrawResult(ImGui::Combo(label, &current_item, &StringVectorComboGetter, const_cast<std::vector<std::string>*>(&items), static_cast<int>(items.size()))).Query();
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
		return DrawResult(ImGui::Checkbox(label, &data)).Query();
	}

	DrawResult InputData<int>::operator()(const char* label, int& data) const
	{
		auto styles = ApplyStyles(GUIState::input_data_styles);
		return DrawResult(ImGui::InputInt(label, &data)).Query();
	}

	DrawResult InputData<int>::operator()(const char* label, int& data, OptionalPrimitive<int> min, OptionalPrimitive<int> max) const
	{
		return InputClampedData(label, data, min, max);
	}

	DrawResult InputData<int>::operator()(const char* label, int& data, const char** names, size_t count)
	{
		auto styles = ApplyStyles(GUIState::input_data_styles);
		return DrawResult(ImGui::Combo(label, &data, names, count)).Query();
	}

	DrawResult InputData<float>::operator()(const char* label, float& data) const
	{
		auto styles = ApplyStyles(GUIState::input_data_styles);
		return DrawResult(ImGui::InputFloat(label, &data)).Query();
	}

	DrawResult InputData<float>::operator()(const char* label, float& data, OptionalPrimitive<float> min, OptionalPrimitive<float> max) const
	{
		return InputClampedData(label, data, min, max);
	}

	DrawResult InputData<double>::operator()(const char* label, double& data) const
	{
		auto styles = ApplyStyles(GUIState::input_data_styles);
		return DrawResult(ImGui::InputDouble(label, &data)).Query();
	}

	DrawResult InputData<double>::operator()(const char* label, double& data, OptionalPrimitive<double> min, OptionalPrimitive<double> max) const
	{
		return InputClampedData(label, data, min, max);
	}

	DrawResult InputData<glm::vec2>::operator()(const char* label, glm::vec2& data) const
	{
		auto styles = ApplyStyles(GUIState::input_data_styles);
		return DrawResult(ImGui::InputFloat2(label, glm::value_ptr(data))).Query();
	}

	DrawResult InputData<glm::vec2>::operator()(const char* label, glm::vec2& data, OptionalPrimitive<float> min, OptionalPrimitive<float> max) const
	{
		return InputClampedData(label, data, min, max);
	}

	DrawResult InputData<glm::vec3>::operator()(const char* label, glm::vec3& data) const
	{
		auto styles = ApplyStyles(GUIState::input_data_styles);
		return DrawResult(ImGui::InputFloat3(label, glm::value_ptr(data))).Query();
	}

	DrawResult InputData<glm::vec3>::operator()(const char* label, glm::vec3& data, OptionalPrimitive<float> min, OptionalPrimitive<float> max) const
	{
		return InputClampedData(label, data, min, max);
	}

	DrawResult InputData<glm::vec4>::operator()(const char* label, glm::vec4& data) const
	{
		auto styles = ApplyStyles(GUIState::input_data_styles);
		return DrawResult(ImGui::InputFloat4(label, glm::value_ptr(data))).Query();
	}

	DrawResult InputData<glm::vec4>::operator()(const char* label, glm::vec4& data, OptionalPrimitive<float> min, OptionalPrimitive<float> max) const
	{
		return InputClampedData(label, data, min, max);
	}

	DrawResult InputData<std::string>::operator()(const char* label, std::string& data) const
	{
		return InputText(label, data);
	}

	DrawResult InputData<Color4>::operator()(const char* label, Color4& data) const
	{
		auto styles = ApplyStyles(GUIState::input_data_styles);
		return DrawResult(ImGui::ColorEdit4(label, data.ValuePtr())).Query();
	}

	// TODO v9.1 InlineWidget: use child with inner tables more often for layout structuring: define utility to defer calls to input-data so that the number of columns can be dynamically computed + SameLine() automatically called for multiple elements in a column.
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
			}

			ImGui::TableNextColumn();
			result |= InputData<float>{}("x1", data.x1);

			ImGui::TableNextColumn();
			result |= InputData<float>{}("x2", data.x2);

			ImGui::TableNextColumn();
			result |= InputData<float>{}("y1", data.y1);

			ImGui::TableNextColumn();
			result |= InputData<float>{}("y2", data.y2);

			ImGui::EndTable();
		}
		return result;
	}

	DrawResult InputData<UVRect>::operator()(const char* label, UVRect& data) const
	{
		const bool header = label && label[0] != '\0';
		DrawResult result;
		if (ImGui::BeginTable("##UVRect", header ? 5 : 4))
		{
			if (header)
			{
				ImGui::TableNextColumn();
				ImGui::Text(label);
			}
			
			ImGui::TableNextColumn();
			result |= InputData<float>{}("x1", data.x1, MakeOpt(0.f), MakeOpt(1.f));

			ImGui::TableNextColumn();
			result |= InputData<float>{}("x2", data.x2, MakeOpt(0.f), MakeOpt(1.f));

			ImGui::TableNextColumn();
			result |= InputData<float>{}("y1", data.y1, MakeOpt(0.f), MakeOpt(1.f));

			ImGui::TableNextColumn();
			result |= InputData<float>{}("y2", data.y2, MakeOpt(0.f), MakeOpt(1.f));

			ImGui::EndTable();
		}
		return result;
	}

	DrawResult InputData<TopSidePadding>::operator()(const char* label, TopSidePadding& data) const
	{
		// TODO v9.1 DescIO should just create an InlineWidget in the second column of form. In fact, Form should be more of a global state machine than object-based. Then, you can dynamically add components to it and just have DescIO call Draw() on it when the field is done.
		InlineWidget widget;

		if (label && label[0] != '\0')
		{
			widget.AddComponent(TextComponent(label));
			widget.AddComponent(VerticalSeparatorComponent());
		}

		widget.AddComponent(InputDataComponent<float>("Left", data.left));
		widget.AddComponent(VerticalSeparatorComponent());
		widget.AddComponent(InputDataComponent<float>("Right", data.right));
		widget.AddComponent(VerticalSeparatorComponent());
		widget.AddComponent(InputDataComponent<float>("Top", data.top));
		return widget.Draw();
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
