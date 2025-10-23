#include "Ellipses.h"

#include "registries/Loader.h"

namespace oly::reg
{
	rendering::Ellipse load_ellipse(TOMLNode node)
	{
		if (LOG.enable.debug)
		{
			auto src = node["source"].value<std::string>();
			OLY_LOG_DEBUG(true, "REG") << LOG.source_info.full_source() << "Parsing ellipse [" << (src ? *src : "") << "]." << LOG.nl;
		}

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

		if (LOG.enable.debug)
		{
			auto src = node["source"].value<std::string>();
			OLY_LOG_DEBUG(true, "REG") << LOG.source_info.full_source() << "Ellipse [" << (src ? *src : "") << "] parsed." << LOG.nl;
		}

		return ellipse;
	}
}
