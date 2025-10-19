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

		params::Ellipse params;
		params.local = load_transform_2d(node["transform"]);

		parse_vec(node["border_inner_color"], params.color.border_inner);
		parse_vec(node["border_outer_color"], params.color.border_outer);
		parse_vec(node["fill_inner_color"], params.color.fill_inner);
		parse_vec(node["fill_outer_color"], params.color.fill_outer);

		parse_float(node["border"], params.dimension.border);
		parse_float(node["border_exp"], params.dimension.border_exp);
		parse_float(node["fill_exp"], params.dimension.fill_exp);
		parse_float(node["rx"], params.dimension.rx);
		parse_float(node["ry"], params.dimension.ry);

		if (LOG.enable.debug)
		{
			auto src = node["source"].value<std::string>();
			OLY_LOG_DEBUG(true, "REG") << LOG.source_info.full_source() << "Ellipse [" << (src ? *src : "") << "] parsed." << LOG.nl;
		}

		return load_ellipse(params);
	}

	rendering::Ellipse load_ellipse(const params::Ellipse& params)
	{
		rendering::Ellipse ellipse;
		ellipse.set_local() = params.local;
		ellipse.ellipse.set_color() = params.color;
		ellipse.ellipse.set_dimension() = params.dimension;
		return ellipse;
	}
}
