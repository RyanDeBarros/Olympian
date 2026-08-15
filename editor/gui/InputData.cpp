#include "InputData.h"

// TODO v9.3 just put at least Color4 in imtk

imtk::item_result imtk::w::simple_widget<oly::editor::Color4>::draw_impl()
{
	id_scope scope(&data);
	auto result = prefix_label(config.label);

	result |= item_result::query(ImGui::ColorEdit4("", data.ValuePtr()));

	result.modified |= check_property(std::make_unique<prop::simple_view<oly::editor::Color4>>(data));
	return result;
}
