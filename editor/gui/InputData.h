#pragma once

#include "core/Types.h"

#include <imtk.hpp>

template<>
struct imtk::w::simple_widget<oly::editor::Color4> : public imtk::w::widget
{
	oly::editor::Color4& data;
	
	struct config_impl
	{
		std::string label;
	} config;

	simple_widget(oly::editor::Color4& data, config_impl config = {}) : data(data), config(std::move(config)) {}

protected:
	item_result draw_impl();
};
