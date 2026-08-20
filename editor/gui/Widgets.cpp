#include "Widgets.h"

static imtk::item_result DrawFloatComponent(std::string_view label, float& data,
	imp::potential<float> min, imp::potential<float> max, float step, float step_fast, const char* format, ImGuiInputTextFlags flags)
{
	return imtk::w::bound_widget<float>(data, { .label = std::string(label), .min = min, .max = max, .step = step, .step_fast = step_fast, .format = format, .flags = flags }).draw();
}

imtk::item_result imtk::w::bound_widget<oly::editor::Rect>::draw_impl()
{
	id_scope scope(&data);
	imtk::item_result result;
	
	// TODO v9.3 do this for glm::vec2/3/4 for individual properties for each float box

	{
		// TODO v9.3 just use bound_widget<float> members, and enforce or initialize min=0/max=1 in UVRect
		imtk::prop::grid::subproperty_scope s;
		result |= DrawFloatComponent("x1", data.x1, config.cfg_x1.min, config.cfg_x1.max, config.cfg_x1.step, config.cfg_x1.step_fast, config.cfg_x1.format, config.cfg_x1.flags);
		imtk::controls::vertical_separator();
		result |= DrawFloatComponent("x2", data.x2, config.cfg_x2.min, config.cfg_x2.max, config.cfg_x2.step, config.cfg_x2.step_fast, config.cfg_x2.format, config.cfg_x2.flags);
		imtk::controls::vertical_separator();
		result |= DrawFloatComponent("y1", data.y1, config.cfg_y1.min, config.cfg_y1.max, config.cfg_y1.step, config.cfg_y1.step_fast, config.cfg_y1.format, config.cfg_y1.flags);
		imtk::controls::vertical_separator();
		result |= DrawFloatComponent("y2", data.y2, config.cfg_y2.min, config.cfg_y2.max, config.cfg_y2.step, config.cfg_y2.step_fast, config.cfg_y2.format, config.cfg_y2.flags);
	}

	result.modified |= imtk::prop::grid::check_property(std::make_unique<imtk::prop::simple_view<oly::editor::Rect>>(data));
	return result;
}

imtk::item_result imtk::w::bound_widget<oly::editor::UVRect>::draw_impl()
{
	id_scope scope(&data);
	imtk::item_result result;

	{
		imtk::prop::grid::subproperty_scope s;
		result |= DrawFloatComponent("x1", data.x1, 0.f, 1.f, config.cfg_x1.step, config.cfg_x1.step_fast, config.cfg_x1.format, config.cfg_x1.flags);
		imtk::controls::vertical_separator();
		result |= DrawFloatComponent("x2", data.x2, 0.f, 1.f, config.cfg_x2.step, config.cfg_x2.step_fast, config.cfg_x2.format, config.cfg_x2.flags);
		imtk::controls::vertical_separator();
		result |= DrawFloatComponent("y1", data.y1, 0.f, 1.f, config.cfg_y1.step, config.cfg_y1.step_fast, config.cfg_y1.format, config.cfg_y1.flags);
		imtk::controls::vertical_separator();
		result |= DrawFloatComponent("y2", data.y2, 0.f, 1.f, config.cfg_y2.step, config.cfg_y2.step_fast, config.cfg_y2.format, config.cfg_y2.flags);
	}

	result.modified |= imtk::prop::grid::check_property(std::make_unique<imtk::prop::simple_view<oly::editor::UVRect>>(data));
	return result;
}

imtk::item_result imtk::w::bound_widget<oly::editor::TopSidePadding>::draw_impl()
{
	id_scope scope(&data);
	imtk::item_result result;

	{
		imtk::prop::grid::subproperty_scope s;
		result |= DrawFloatComponent("left", data.left, config.cfg_left.min, config.cfg_left.max, config.cfg_left.step, config.cfg_left.step_fast, config.cfg_left.format, config.cfg_left.flags);
		imtk::controls::vertical_separator();
		result |= DrawFloatComponent("right", data.right, config.cfg_right.min, config.cfg_right.max, config.cfg_right.step, config.cfg_right.step_fast, config.cfg_right.format, config.cfg_right.flags);
		imtk::controls::vertical_separator();
		result |= DrawFloatComponent("top", data.top, config.cfg_top.min, config.cfg_top.max, config.cfg_top.step, config.cfg_top.step_fast, config.cfg_top.format, config.cfg_top.flags);
	}

	result.modified |= imtk::prop::grid::check_property(std::make_unique<imtk::prop::simple_view<oly::editor::TopSidePadding>>(data));
	return result;
}
