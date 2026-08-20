#pragma once

#include "core/Types.h"

#include <imtk.hpp>

template<>
struct imtk::w::bound_widget<oly::editor::Rect> : public widget
{
	oly::editor::Rect& data;

	struct config_impl
	{
		struct sub_config
		{
			imp::potential<float> min = imp::nullpotential;
			imp::potential<float> max = imp::nullpotential;

			float step = 0.f;
			float step_fast = 0.f;
			const char* format = "%.3f";
			ImGuiInputTextFlags flags = 0;
		};

		sub_config cfg_x1;
		sub_config cfg_x2;
		sub_config cfg_y1;
		sub_config cfg_y2;
	} config;

	bound_widget(oly::editor::Rect& data, config_impl config = {}) : data(data), config(std::move(config)) {}

protected:
	item_result draw_impl() override;
};

template<>
struct imtk::w::bound_widget<oly::editor::UVRect> : public widget
{
	oly::editor::UVRect& data;

	struct config_impl
	{
		struct sub_config
		{
			float step = 0.f;
			float step_fast = 0.f;
			const char* format = "%.3f";
			ImGuiInputTextFlags flags = 0;
		};

		sub_config cfg_x1;
		sub_config cfg_x2;
		sub_config cfg_y1;
		sub_config cfg_y2;
	} config;

	bound_widget(oly::editor::UVRect& data, config_impl config = {}) : data(data), config(std::move(config)) {}

protected:
	item_result draw_impl() override;
};

template<>
struct imtk::w::bound_widget<oly::editor::TopSidePadding> : public widget
{
	oly::editor::TopSidePadding& data;

	struct config_impl
	{
		struct sub_config
		{
			imp::potential<float> min = imp::nullpotential;
			imp::potential<float> max = imp::nullpotential;

			float step = 0.f;
			float step_fast = 0.f;
			const char* format = "%.3f";
			ImGuiInputTextFlags flags = 0;
		};

		sub_config cfg_left;
		sub_config cfg_right;
		sub_config cfg_top;
	} config;

	bound_widget(oly::editor::TopSidePadding& data, config_impl config = {}) : data(data), config(std::move(config)) {}

protected:
	item_result draw_impl() override;
};
