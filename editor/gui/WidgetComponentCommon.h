#pragma once

#include "WidgetComponent.h"

#include "gui/ImGuiWrapper.h"

namespace oly::editor::comp
{
	extern gui::WidgetComponent Text(const char* label);

	template<typename T, typename... Args>
	extern gui::WidgetComponent InputData(const char* label, T& data, Args&&... args)
	{
		gui::WidgetComponent c;
		c.draw = [label, &data, ... args = std::forward<Args>(args)]() mutable  -> imtk::item_result {
			imtk::id_scope scope(&data);
			return gui::InputData<T>{}(label, data, std::forward<Args>(args)...);
		};
		return c;
	}

	template<typename T, typename... Args>
	extern gui::WidgetComponent LabelInputData(const char* label, const char* data_label, T& data, Args&&... args)
	{
		gui::WidgetComponent c;
		c.draw = [label, data_label, &data, ... args = std::forward<Args>(args)]() mutable  -> imtk::item_result {
			imtk::id_scope scope(&data);
			ImGui::TextUnformatted(label);
			auto result = imtk::item_result::query(false);
			ImGui::SameLine();
			return result | gui::InputData<T>{}(data_label, data, std::forward<Args>(args)...);
		};
		return c;
	}

	template<typename T, typename... Args>
	extern gui::WidgetComponent LabelInputDataSep(const char* label, const char* data_label, T& data, Args&&... args)
	{
		gui::WidgetComponent c;
		c.draw = [label, data_label, &data, ... args = std::forward<Args>(args)]() mutable  -> imtk::item_result {
			imtk::id_scope scope(&data);
			imtk::controls::vertical_separator();
			ImGui::TextUnformatted(label);
			imtk::item_result result = imtk::item_result::query(false);
			ImGui::SameLine();
			return result | gui::InputData<T>{}(data_label, data, std::forward<Args>(args)...);
			};
		return c;
	}

	extern gui::WidgetComponent Generic(std::function<imtk::item_result()> draw);
}
