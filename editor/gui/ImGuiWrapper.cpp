#include "ImGuiWrapper.h"

#include "gui/GUIState.h"
#include "gui/InlineWidget.h"
#include "gui/WidgetComponentCommon.h"
#include "gui/properties/PropertyGrid.h"

#include <imgui_internal.h>

namespace oly::editor::gui
{
	DrawResult InputData<bool>::operator()(const char* label, bool& data) const
	{
		auto _ = GUIState::input_data_styles.apply();
		DrawResult result = DrawResult(ImGui::Checkbox(label, &data)).Query();
		result |= PropertyGrid::Value::CheckProperty(std::make_unique<imtk::prop::simple_view<bool>>(data));
		return result;
	}

	DrawResult InputData<int>::operator()(const char* label, int& data) const
	{
		auto _ = GUIState::input_data_styles.apply();
		DrawResult result = DrawResult(ImGui::InputInt(label, &data)).Query();
		result |= PropertyGrid::Value::CheckProperty(std::make_unique<imtk::prop::simple_view<int>>(data));
		return result;
	}

	DrawResult InputData<int>::operator()(const char* label, int& data, OptionalPrimitive<int> min, OptionalPrimitive<int> max) const
	{
		return InputClampedData(label, data, min, max);
	}

	DrawResult InputData<int>::operator()(const char* label, int& data, imtk::label_span_registry::handle names)
	{
		auto _ = GUIState::input_data_styles.apply();
		DrawResult result = DrawResult(ImGui::Combo(label, &data, &imtk::label_span_registry::combo_getter, &names, imtk::label_span_registry::count(names))).Query();
		result |= PropertyGrid::Value::CheckProperty(std::make_unique<imtk::prop::combo_view>(data, names));
		return result;
	}

	DrawResult InputData<float>::operator()(const char* label, float& data) const
	{
		auto _ = GUIState::input_data_styles.apply();
		DrawResult result = DrawResult(ImGui::InputFloat(label, &data)).Query();
		result |= PropertyGrid::Value::CheckProperty(std::make_unique<imtk::prop::simple_view<float>>(data));
		return result;
	}

	DrawResult InputData<float>::operator()(const char* label, float& data, OptionalPrimitive<float> min, OptionalPrimitive<float> max) const
	{
		return InputClampedData(label, data, min, max);
	}

	DrawResult InputData<double>::operator()(const char* label, double& data) const
	{
		auto _ = GUIState::input_data_styles.apply();
		DrawResult result = DrawResult(ImGui::InputDouble(label, &data)).Query();
		result |= PropertyGrid::Value::CheckProperty(std::make_unique<imtk::prop::simple_view<double>>(data));
		return result;
	}

	DrawResult InputData<double>::operator()(const char* label, double& data, OptionalPrimitive<double> min, OptionalPrimitive<double> max) const
	{
		return InputClampedData(label, data, min, max);
	}

	DrawResult InputData<glm::vec2>::operator()(const char* label, glm::vec2& data) const
	{
		auto _ = GUIState::input_data_styles.apply();
		DrawResult result = DrawResult(ImGui::InputFloat2(label, glm::value_ptr(data))).Query();
		result |= PropertyGrid::Value::CheckProperty(std::make_unique<imtk::prop::simple_view<glm::vec2>>(data));
		return result;
	}

	DrawResult InputData<glm::vec2>::operator()(const char* label, glm::vec2& data, OptionalPrimitive<float> min, OptionalPrimitive<float> max) const
	{
		return InputClampedData(label, data, min, max);
	}

	DrawResult InputData<glm::vec3>::operator()(const char* label, glm::vec3& data) const
	{
		auto _ = GUIState::input_data_styles.apply();
		DrawResult result = DrawResult(ImGui::InputFloat3(label, glm::value_ptr(data))).Query();
		result |= PropertyGrid::Value::CheckProperty(std::make_unique<imtk::prop::simple_view<glm::vec3>>(data));
		return result;
	}

	DrawResult InputData<glm::vec3>::operator()(const char* label, glm::vec3& data, OptionalPrimitive<float> min, OptionalPrimitive<float> max) const
	{
		return InputClampedData(label, data, min, max);
	}

	DrawResult InputData<glm::vec4>::operator()(const char* label, glm::vec4& data) const
	{
		auto _ = GUIState::input_data_styles.apply();
		DrawResult result = DrawResult(ImGui::InputFloat4(label, glm::value_ptr(data))).Query();
		result |= PropertyGrid::Value::CheckProperty(std::make_unique<imtk::prop::simple_view<glm::vec4>>(data));
		return result;
	}

	DrawResult InputData<glm::vec4>::operator()(const char* label, glm::vec4& data, OptionalPrimitive<float> min, OptionalPrimitive<float> max) const
	{
		return InputClampedData(label, data, min, max);
	}

	DrawResult InputData<std::string>::operator()(const char* label, std::string& data) const
	{
		auto _ = GUIState::input_data_styles.apply();
		DrawResult result = DrawResult(imtk::controls::input_text(label, data)).Query();
		result |= PropertyGrid::Value::CheckProperty(std::make_unique<imtk::prop::simple_view<std::string>>(data));
		return result;
	}

	DrawResult InputData<Color4>::operator()(const char* label, Color4& data) const
	{
		auto _ = GUIState::input_data_styles.apply();
		DrawResult result = DrawResult(ImGui::ColorEdit4(label, data.ValuePtr())).Query();
		result |= PropertyGrid::Value::CheckProperty(std::make_unique<imtk::prop::simple_view<Color4>>(data));
		return result;
	}
}
