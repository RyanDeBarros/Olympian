#include "Ellipses.h"

#include "core/base/Context.h"
#include "registries/Loader.h"

namespace oly::reg
{
	// TODO remove unused UNREGISTERED_* error codes

	rendering::Ellipse oly::reg::load_ellipse(const TOMLNode& node)
	{
		rendering::Ellipse ellipse = context::ellipse();

		ellipse.set_local() = load_transform_2d(node, "transform");

		auto& color = ellipse.ellipse.set_color();
		auto& dimension = ellipse.ellipse.set_dimension();
		parse_vec4(node, "border inner color", color.border_inner);
		parse_vec4(node, "border outer color", color.border_outer);
		parse_vec4(node, "fill inner color", color.fill_inner);
		parse_vec4(node, "fill outer color", color.fill_outer);
		parse_float(node, "border", dimension.border);
		parse_float(node, "border exp", dimension.border_exp);
		parse_float(node, "fill exp", dimension.fill_exp);
		parse_float(node, "width", dimension.width);
		parse_float(node, "height", dimension.height);

		return ellipse;
	}
}
