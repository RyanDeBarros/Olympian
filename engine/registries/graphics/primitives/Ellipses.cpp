#include "Ellipses.h"

#include "core/base/Context.h"
#include "registries/Loader.h"

namespace oly::reg
{
	rendering::Ellipse oly::reg::load_ellipse(const TOMLNode& node)
	{
		params::Ellipse params;
		params.local = load_transform_2d(node, "transform");

		glm::vec4 v4;
		if (parse_vec4(node, "border inner color", v4))
			params.color.border_inner = v4;
		if (parse_vec4(node, "border outer color", v4))
			params.color.border_outer = v4;
		if (parse_vec4(node, "fill inner color", v4))
			params.color.fill_inner = v4;
		if (parse_vec4(node, "fill outer color", v4))
			params.color.fill_outer = v4;

		float v1;
		if (parse_float(node, "border", v1))
			params.dimension.border = v1;
		if (parse_float(node, "border exp", v1))
			params.dimension.border_exp = v1;
		if (parse_float(node, "fill exp", v1))
			params.dimension.fill_exp = v1;
		if (parse_float(node, "width", v1))
			params.dimension.width = v1;
		if (parse_float(node, "height", v1))
			params.dimension.height = v1;

		return load_ellipse(params);
	}

	rendering::Ellipse load_ellipse(const params::Ellipse& params)
	{
		rendering::Ellipse ellipse = context::ellipse();

		ellipse.set_local() = params.local;

		auto& color = ellipse.ellipse.set_color();
		if (params.color.border_inner)
			color.border_inner = params.color.border_inner.value();
		if (params.color.border_outer)
			color.border_outer = params.color.border_outer.value();
		if (params.color.fill_inner)
			color.fill_inner = params.color.fill_inner.value();
		if (params.color.fill_outer)
			color.fill_outer = params.color.fill_outer.value();

		auto& dimension = ellipse.ellipse.set_dimension();
		if (params.dimension.width)
			dimension.width = params.dimension.width.value();
		if (params.dimension.height)
			dimension.height = params.dimension.height.value();
		if (params.dimension.border)
			dimension.border = params.dimension.border.value();
		if (params.dimension.border_exp)
			dimension.border_exp = params.dimension.border_exp.value();
		if (params.dimension.fill_exp)
			dimension.fill_exp = params.dimension.fill_exp.value();

		return ellipse;
	}
}
