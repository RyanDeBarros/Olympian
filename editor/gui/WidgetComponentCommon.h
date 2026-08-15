#pragma once

#include "WidgetComponent.h"

#include "gui/ImGuiWrapper.h"

namespace oly::editor::comp
{
	// TODO v9.3 use actual simple_widget classes instead of these methods
	
	template<typename T, typename... Args>
	extern std::unique_ptr<imtk::w::widget> InputData(const char* label, T& data, Args&&... args)
	{
		return std::make_unique<imtk::w::generic_widget>([label, &data, ... args = std::forward<Args>(args)]() mutable  -> imtk::item_result {
			imtk::id_scope scope(&data);
			return gui::InputData<T>{}(label, data, std::forward<Args>(args)...);
		});
	}

	template<typename T, typename... Args>
	extern std::unique_ptr<imtk::w::widget> LabelInputData(const char* label, const char* data_label, T& data, Args&&... args)
	{
		return std::make_unique<imtk::w::generic_widget>([label, data_label, &data, ... args = std::forward<Args>(args)]() mutable  -> imtk::item_result {
			imtk::id_scope scope(&data);
			ImGui::TextUnformatted(label);
			auto result = imtk::item_result::query(false);
			ImGui::SameLine();
			return result | gui::InputData<T>{}(data_label, data, std::forward<Args>(args)...);
		});
	}

	template<typename T, typename... Args>
	extern std::unique_ptr<imtk::w::widget> LabelInputDataSep(const char* label, const char* data_label, T& data, Args&&... args)
	{
		return std::make_unique<imtk::w::generic_widget>([label, data_label, &data, ... args = std::forward<Args>(args)]() mutable  -> imtk::item_result {
			imtk::id_scope scope(&data);
			imtk::controls::vertical_separator();
			ImGui::TextUnformatted(label);
			imtk::item_result result = imtk::item_result::query(false);
			ImGui::SameLine();
			return result | gui::InputData<T>{}(data_label, data, std::forward<Args>(args)...);
		});
	}
}
