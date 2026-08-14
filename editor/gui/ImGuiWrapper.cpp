#include "ImGuiWrapper.h"

#include "gui/GUIState.h"
#include "gui/InlineWidget.h"
#include "gui/WidgetComponentCommon.h"
#include "gui/properties/PropertyGrid.h"

#include <imgui_internal.h>

namespace oly::editor::gui
{
	imtk::item_result InputData<bool>::operator()(const char* label, bool& data) const
	{
		auto _ = GUIState::input_data_styles.apply();
		auto result = imtk::item_result::query(ImGui::Checkbox(label, &data));
		result.modified |= PropertyGrid::Value::CheckProperty(std::make_unique<imtk::prop::simple_view<bool>>(data));
		return result;
	}

	imtk::item_result InputData<int>::operator()(const char* label, int& data) const
	{
		auto _ = GUIState::input_data_styles.apply();
		auto result = imtk::item_result::query(ImGui::InputInt(label, &data));
		result.modified |= PropertyGrid::Value::CheckProperty(std::make_unique<imtk::prop::simple_view<int>>(data));
		return result;
	}

	imtk::item_result InputData<int>::operator()(const char* label, int& data, imp::potential<int> min, imp::potential<int> max) const
	{
		return InputClampedData(label, data, min, max);
	}

	imtk::item_result InputData<int>::operator()(const char* label, int& data, imtk::label_span_registry::handle names)
	{
		auto _ = GUIState::input_data_styles.apply();
		auto result = imtk::item_result::query(ImGui::Combo(label, &data, &imtk::label_span_registry::combo_getter, &names, imtk::label_span_registry::count(names)));
		result.modified |= PropertyGrid::Value::CheckProperty(std::make_unique<imtk::prop::combo_view>(data, names));
		return result;
	}

	imtk::item_result InputData<float>::operator()(const char* label, float& data) const
	{
		auto _ = GUIState::input_data_styles.apply();
		auto result = imtk::item_result::query(ImGui::InputFloat(label, &data));
		result.modified |= PropertyGrid::Value::CheckProperty(std::make_unique<imtk::prop::simple_view<float>>(data));
		return result;
	}

	imtk::item_result InputData<float>::operator()(const char* label, float& data, imp::potential<float> min, imp::potential<float> max) const
	{
		return InputClampedData(label, data, min, max);
	}

	imtk::item_result InputData<double>::operator()(const char* label, double& data) const
	{
		auto _ = GUIState::input_data_styles.apply();
		auto result = imtk::item_result::query(ImGui::InputDouble(label, &data));
		result.modified |= PropertyGrid::Value::CheckProperty(std::make_unique<imtk::prop::simple_view<double>>(data));
		return result;
	}

	imtk::item_result InputData<double>::operator()(const char* label, double& data, imp::potential<double> min, imp::potential<double> max) const
	{
		return InputClampedData(label, data, min, max);
	}

	imtk::item_result InputData<glm::vec2>::operator()(const char* label, glm::vec2& data) const
	{
		auto _ = GUIState::input_data_styles.apply();
		auto result = imtk::item_result::query(ImGui::InputFloat2(label, glm::value_ptr(data)));
		result.modified |= PropertyGrid::Value::CheckProperty(std::make_unique<imtk::prop::simple_view<glm::vec2>>(data));
		return result;
	}

	imtk::item_result InputData<glm::vec2>::operator()(const char* label, glm::vec2& data, imp::potential<float> min, imp::potential<float> max) const
	{
		return InputClampedData(label, data, min, max);
	}

	imtk::item_result InputData<glm::vec3>::operator()(const char* label, glm::vec3& data) const
	{
		auto _ = GUIState::input_data_styles.apply();
		auto result = imtk::item_result::query(ImGui::InputFloat3(label, glm::value_ptr(data)));
		result.modified |= PropertyGrid::Value::CheckProperty(std::make_unique<imtk::prop::simple_view<glm::vec3>>(data));
		return result;
	}

	imtk::item_result InputData<glm::vec3>::operator()(const char* label, glm::vec3& data, imp::potential<float> min, imp::potential<float> max) const
	{
		return InputClampedData(label, data, min, max);
	}

	imtk::item_result InputData<glm::vec4>::operator()(const char* label, glm::vec4& data) const
	{
		auto _ = GUIState::input_data_styles.apply();
		auto result = imtk::item_result::query(ImGui::InputFloat4(label, glm::value_ptr(data)));
		result.modified |= PropertyGrid::Value::CheckProperty(std::make_unique<imtk::prop::simple_view<glm::vec4>>(data));
		return result;
	}

	imtk::item_result InputData<glm::vec4>::operator()(const char* label, glm::vec4& data, imp::potential<float> min, imp::potential<float> max) const
	{
		return InputClampedData(label, data, min, max);
	}

	imtk::item_result InputData<std::string>::operator()(const char* label, std::string& data) const
	{
		auto _ = GUIState::input_data_styles.apply();
		auto result = imtk::item_result::query(imtk::controls::input_text(label, data));
		result.modified |= PropertyGrid::Value::CheckProperty(std::make_unique<imtk::prop::simple_view<std::string>>(data));
		return result;
	}

	imtk::item_result InputData<Color4>::operator()(const char* label, Color4& data) const
	{
		auto _ = GUIState::input_data_styles.apply();
		auto result = imtk::item_result::query(ImGui::ColorEdit4(label, data.ValuePtr()));
		result.modified |= PropertyGrid::Value::CheckProperty(std::make_unique<imtk::prop::simple_view<Color4>>(data));
		return result;
	}
}
