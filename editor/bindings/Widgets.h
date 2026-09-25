#pragma once

#include "core/Types.h"

#include <imtk.hpp>

namespace imtk::w
{
	template<>
	struct bound_widget<oly::editor::Rect> : public widget
	{
		oly::editor::Rect& data;
		bound_widget<float> x1;
		bound_widget<float> x2;
		bound_widget<float> y1;
		bound_widget<float> y2;

		bound_widget(oly::editor::Rect& data);

	protected:
		item_result draw_impl() override;
	};

	template<>
	struct bound_widget<oly::editor::UVRect> : public widget
	{
		oly::editor::UVRect& data;
		bound_widget<float> x1;
		bound_widget<float> x2;
		bound_widget<float> y1;
		bound_widget<float> y2;

		bound_widget(oly::editor::UVRect& data);

	protected:
		item_result draw_impl() override;
	};

	template<>
	struct bound_widget<oly::editor::TopSidePadding> : public widget
	{
		oly::editor::TopSidePadding& data;
		bound_widget<float> left;
		bound_widget<float> right;
		bound_widget<float> top;

		bound_widget(oly::editor::TopSidePadding& data);

	protected:
		item_result draw_impl() override;
	};
}

