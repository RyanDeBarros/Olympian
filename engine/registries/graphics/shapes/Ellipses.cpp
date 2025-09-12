#include "Ellipses.h"

#include "registries/Loader.h"

namespace oly::reg
{
	rendering::Ellipse load_ellipse(const TOMLNode& node)
	{
		if (LOG.enable.debug)
		{
			auto src = node["source"].value<std::string>();
			OLY_LOG_DEBUG(true, "REG") << LOG.source_info.full_source() << "Parsing ellipse [" << (src ? *src : "") << "]." << LOG.nl;
		}

		params::Ellipse params;
		params.local = load_transform_2d(node, "transform");

		glm::vec4 v4;
		if (parse_vec(node["border inner color"].as_array(), v4))
			params.color.border_inner = v4;
		if (parse_vec(node["border outer color"].as_array(), v4))
			params.color.border_outer = v4;
		if (parse_vec(node["fill inner color"].as_array(), v4))
			params.color.fill_inner = v4;
		if (parse_vec(node["fill outer color"].as_array(), v4))
			params.color.fill_outer = v4;

		float v1;
		if (parse_float(node, "border", v1))
			params.dimension.border = v1;
		if (parse_float(node, "border exp", v1))
			params.dimension.border_exp = v1;
		if (parse_float(node, "fill exp", v1))
			params.dimension.fill_exp = v1;
		if (parse_float(node, "rx", v1))
			params.dimension.rx = v1;
		if (parse_float(node, "ry", v1))
			params.dimension.ry = v1;

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
		if (params.dimension.rx)
			dimension.rx = params.dimension.rx.value();
		if (params.dimension.ry)
			dimension.ry = params.dimension.ry.value();
		if (params.dimension.border)
			dimension.border = params.dimension.border.value();
		if (params.dimension.border_exp)
			dimension.border_exp = params.dimension.border_exp.value();
		if (params.dimension.fill_exp)
			dimension.fill_exp = params.dimension.fill_exp.value();

		return ellipse;
	}
}
