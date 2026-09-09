#include "Widgets.h"

namespace imtk::w
{
	static item_result DrawFloatComponent(std::string_view label, float& data,
		imp::potential<float> min, imp::potential<float> max, float step, float step_fast, const char* format, ImGuiInputTextFlags flags)
	{
		return bound_widget<float>(data, { .label = std::string(label), .min = min, .max = max, .step = step, .step_fast = step_fast, .format = format, .flags = flags }).draw();
	}

	bound_widget<oly::editor::Rect>::bound_widget(oly::editor::Rect& data)
		: data(data), x1(data.x1), x2(data.x2), y1(data.y1), y2(data.y2)
	{
		x1.config.label = "x1";
		x2.config.label = "x2";
		y1.config.label = "y1";
		y2.config.label = "y2";
	}

	item_result bound_widget<oly::editor::Rect>::draw_impl()
	{
		id_scope scope(&data);
		item_result result;
	
		// TODO v9.3 do this for glm::vec2/3/4 for individual properties for each float box ?

		if (auto s = prop::grid::subproperty_scope())
		{
			widget* subwidgets[] = { &x1, &x2, &y1, &y2 };
			result |= bound_widget_row(subwidgets, { .separators = true }).draw();
		}

		result.modified |= prop::grid::check_property(std::make_unique<prop::simple_view<oly::editor::Rect>>(data));
		return result;
	}

	bound_widget<oly::editor::UVRect>::bound_widget(oly::editor::UVRect& data)
		: data(data), x1(data.x1), x2(data.x2), y1(data.y1), y2(data.y2)
	{
		x1.config.label = "x1";
		x1.config.min = 0.f;
		x1.config.max = 1.f;
		x2.config.label = "x2";
		x2.config.min = 0.f;
		x2.config.max = 1.f;
		y1.config.label = "y1";
		y1.config.min = 0.f;
		y1.config.max = 1.f;
		y2.config.label = "y2";
		y2.config.min = 0.f;
		y2.config.max = 1.f;
	}

	item_result bound_widget<oly::editor::UVRect>::draw_impl()
	{
		id_scope scope(&data);
		item_result result;

		if (auto s = prop::grid::subproperty_scope())
		{
			widget* subwidgets[] = { &x1, &x2, &y1, &y2 };
			result |= bound_widget_row(subwidgets, { .separators = true }).draw();
		}

		result.modified |= prop::grid::check_property(std::make_unique<prop::simple_view<oly::editor::UVRect>>(data));
		return result;
	}

	bound_widget<oly::editor::TopSidePadding>::bound_widget(oly::editor::TopSidePadding& data)
		: data(data), left(data.left), right(data.right), top(data.top)
	{
		left.config.label = "left";
		right.config.label = "right";
		top.config.label = "top";
	}

	item_result bound_widget<oly::editor::TopSidePadding>::draw_impl()
	{
		id_scope scope(&data);
		item_result result;

		if (auto s = prop::grid::subproperty_scope())
		{
			widget* subwidgets[] = { &left, &right, &top };
			result |= bound_widget_row(subwidgets, { .separators = true }).draw();
		}

		result.modified |= prop::grid::check_property(std::make_unique<prop::simple_view<oly::editor::TopSidePadding>>(data));
		return result;
	}
}
