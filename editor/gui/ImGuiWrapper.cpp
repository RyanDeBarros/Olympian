#include "ImGuiWrapper.h"

#include "gui/GUIState.h"
#include "gui/WidgetComponentCommon.h"
#include "gui/properties/PropertyGroup.h"
#include "gui/properties/PropertyViews.h"

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
		DrawResult result = DrawResult(ImGui::Checkbox(label, &data)).Query();
		result.SetDirty(PropertyGroup::Append(std::make_unique<prop::PrimitivePropertyView<bool>>(data)));
		return result;
	}

	DrawResult InputData<int>::operator()(const char* label, int& data) const
	{
		auto styles = ApplyStyles(GUIState::input_data_styles);
		DrawResult result = DrawResult(ImGui::InputInt(label, &data)).Query();
		result.SetDirty(PropertyGroup::Append(std::make_unique<prop::PrimitivePropertyView<int>>(data)));
		return result;
	}

	DrawResult InputData<int>::operator()(const char* label, int& data, OptionalPrimitive<int> min, OptionalPrimitive<int> max) const
	{
		return InputClampedData(label, data, min, max);
	}

	DrawResult InputData<int>::operator()(const char* label, int& data, LabelSpanRegistry::Handle names)
	{
		auto styles = ApplyStyles(GUIState::input_data_styles);
		DrawResult result = DrawResult(ImGui::Combo(label, &data, &LabelSpanRegistry::ComboGetter, &names, LabelSpanRegistry::Count(names))).Query();
		result.SetDirty(PropertyGroup::Append(std::make_unique<prop::ComboPropertyView>(data, names)));
		return result;
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
}
