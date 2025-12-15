#include "Ellipses.h"

#include "assets/Loader.h"

namespace oly::assets
{
	rendering::Ellipse load_ellipse(TOMLNode node, const char* source)
	{
		_OLY_ENGINE_LOG_DEBUG("ASSETS") << "Parsing ellipse [" << (source ? source : "") << "]..." << LOG.nl;

		rendering::Ellipse ellipse;
		ellipse.transformer = load_transformer_2d(node["transformer"]);

		auto& color = ellipse.ellipse.set_color();
		parse_vec(node["border_inner_color"], color.border_inner);
		parse_vec(node["border_outer_color"], color.border_outer);
		parse_vec(node["fill_inner_color"], color.fill_inner);
		parse_vec(node["fill_outer_color"], color.fill_outer);

		auto& dimension = ellipse.ellipse.set_dimension();
		parse_float(node["border"], dimension.border);
		parse_float(node["border_exp"], dimension.border_exp);
		parse_float(node["fill_exp"], dimension.fill_exp);
		parse_float(node["rx"], dimension.rx);
		parse_float(node["ry"], dimension.ry);

		_OLY_ENGINE_LOG_DEBUG("ASSETS") << "...Ellipse [" << (source ? source : "") << "] parsed." << LOG.nl;

		return ellipse;
	}
}
