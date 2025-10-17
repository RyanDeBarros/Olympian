#include "Polygons.h"

#include "core/cmath/Triangulation.h"
#include "registries/Loader.h"

namespace oly::reg
{
	rendering::Polygon load_polygon(const TOMLNode& node)
	{
		if (LOG.enable.debug)
		{
			auto src = node["source"].value<std::string>();
			OLY_LOG_DEBUG(true, "REG") << LOG.source_info.full_source() << "Parsing polygon [" << (src ? *src : "") << "]." << LOG.nl;
		}

		params::Polygon params;

		params.local = load_transform_2d(node["transform"]);

		auto toml_points = node["points"].as_array();
		if (toml_points)
		{
			size_t pt_idx = 0;
			for (auto& toml_point : *toml_points)
			{
				glm::vec2 pt;
				if (parse_vec((TOMLNode)toml_point, pt))
					params.points.push_back(pt);
				else
					OLY_LOG_WARNING(true, "REG") << LOG.source_info.full_source() << "Cannot convert polygon point #" << pt_idx << " to vec2." << LOG.nl;
				++pt_idx;
			}
		}

		auto toml_colors = node["colors"].as_array();
		if (toml_colors)
		{
			size_t color_idx = 0;
			for (auto& toml_color : *toml_colors)
			{
				glm::vec4 col;
				if (parse_vec((TOMLNode)toml_color, col))
					params.colors.push_back(col);
				else
					OLY_LOG_WARNING(true, "REG") << LOG.source_info.full_source() << "Cannot convert polygon point color #" << color_idx << " to vec4." << LOG.nl;
				++color_idx;
			}
		}

		if (LOG.enable.debug)
		{
			auto src = node["source"].value<std::string>();
			OLY_LOG_DEBUG(true, "REG") << LOG.source_info.full_source() << "Polygon [" << (src ? *src : "") << "] parsed." << LOG.nl;
		}

		return load_polygon(std::move(params));
	}

	rendering::Polygon load_polygon(const params::Polygon& params)
	{
		rendering::Polygon polygon;

		polygon.set_local() = params.local;
		polygon.set_points() = params.points;
		polygon.set_colors() = params.colors;

		return polygon;
	}

	rendering::Polygon load_polygon(params::Polygon&& params)
	{
		rendering::Polygon polygon;

		polygon.set_local() = params.local;
		polygon.set_points() = std::move(params.points);
		polygon.set_colors() = std::move(params.colors);

		return polygon;
	}

	rendering::PolyComposite load_poly_composite(const TOMLNode& node)
	{
		if (LOG.enable.debug)
		{
			auto src = node["source"].value<std::string>();
			OLY_LOG_DEBUG(true, "REG") << LOG.source_info.full_source() << "Parsing poly composite [" << (src ? *src : "") << "]." << LOG.nl;
		}

		params::PolyComposite params;

		params.local = load_transform_2d(node["transform"]);

		auto toml_method = node["method"].value<std::string>();
		if (toml_method)
		{
			const std::string& method = toml_method.value();
			if (method == "ngon")
			{
				params::PolyComposite::NGonMethod method;

				auto toml_points = node["points"].as_array();
				if (toml_points)
				{
					size_t pt_idx = 0;
					for (auto& toml_point : *toml_points)
					{
						glm::vec2 pt;
						if (parse_vec((TOMLNode)toml_point, pt))
							method.points.push_back(pt);
						else
							OLY_LOG_WARNING(true, "REG") << LOG.source_info.full_source() << "Cannot convert poly composite point #" << pt_idx << " to vec2." << LOG.nl;
						++pt_idx;
					}
				}

				auto toml_fill_colors = node["colors"].as_array();
				if (toml_fill_colors)
				{
					size_t color_idx = 0;
					for (auto& toml_color : *toml_fill_colors)
					{
						glm::vec4 col;
						if (parse_vec((TOMLNode)toml_color, col))
							method.colors.push_back(col);
						else
							OLY_LOG_WARNING(true, "REG") << LOG.source_info.full_source() << "Cannot convert poly composite point color #" << color_idx << " to vec4." << LOG.nl;
						++color_idx;
					}
				}

				params.method = std::move(method);
			}
			else if (method == "bordered_ngon")
			{
				params::PolyComposite::BorderedNGonMethod method;

				auto toml_points = node["points"].as_array();
				if (toml_points)
				{
					size_t pt_idx = 0;
					for (auto& toml_point : *toml_points)
					{
						glm::vec2 pt;
						if (parse_vec((TOMLNode)toml_point, pt))
							method.ngon_base.points.push_back(pt);
						else
							OLY_LOG_WARNING(true, "REG") << LOG.source_info.full_source() << "Cannot convert poly composite point #" << pt_idx << " to vec2." << LOG.nl;
						++pt_idx;
					}
				}

				auto toml_fill_colors = node["fill_colors"].as_array();
				if (toml_fill_colors)
				{
					size_t color_idx = 0;
					for (auto& toml_color : *toml_fill_colors)
					{
						glm::vec4 col;
						if (parse_vec((TOMLNode)toml_color, col))
							method.ngon_base.fill_colors.push_back(col);
						else
							OLY_LOG_WARNING(true, "REG") << LOG.source_info.full_source() << "Cannot convert poly composite fill color #" << color_idx << " to vec4." << LOG.nl;
						++color_idx;
					}
				}

				auto toml_border_colors = node["border_colors"].as_array();
				if (toml_border_colors)
				{
					size_t color_idx = 0;
					for (auto& toml_color : *toml_border_colors)
					{
						glm::vec4 col;
						if (parse_vec((TOMLNode)toml_color, col))
							method.ngon_base.border_colors.push_back(col);
						else
							OLY_LOG_WARNING(true, "REG") << LOG.source_info.full_source() << "Cannot convert poly composite border color #" << color_idx << " to vec4." << LOG.nl;
						++color_idx;
					}
				}

				parse_float(node["border_width"], method.ngon_base.border_width);

				if (auto border_pivot = node["border_pivot"])
				{
					if (auto str_border_pivot = border_pivot.value<std::string>())
					{
						const std::string& str = str_border_pivot.value();
						if (str == "outer")
							method.ngon_base.border_pivot = cmath::BorderPivot::OUTER;
						else if (str == "middle")
							method.ngon_base.border_pivot = cmath::BorderPivot::MIDDLE;
						else if (str == "inner")
							method.ngon_base.border_pivot = cmath::BorderPivot::INNER;
						else
							OLY_LOG_WARNING(true, "REG") << LOG.source_info.full_source() << "Unrecognized border pivot named value \"" << str << "\"." << LOG.nl;
					}
					else
						parse_float(border_pivot, method.ngon_base.border_pivot.v);
				}

				params.method = std::move(method);
			}
			else if (method == "convex_decomposition")
			{
				params::PolyComposite::ConvexDecompositionMethod method;

				auto toml_points = node["points"].as_array();
				if (toml_points)
				{
					size_t pt_idx = 0;
					for (auto& toml_point : *toml_points)
					{
						glm::vec2 pt;
						if (parse_vec((TOMLNode)toml_point, pt))
							method.points.push_back(pt);
						else
							OLY_LOG_WARNING(true, "REG") << LOG.source_info.full_source() << "Cannot convert poly composite point #" << pt_idx << " to vec2." << LOG.nl;
						++pt_idx;
					}
				}

				params.method = std::move(method);
			}
		}

		if (LOG.enable.debug)
		{
			auto src = node["source"].value<std::string>();
			OLY_LOG_DEBUG(true, "REG") << LOG.source_info.full_source() << "Poly composite [" << (src ? *src : "") << "] parsed." << LOG.nl;
		}

		return load_poly_composite(std::move(params));
	}

	rendering::PolyComposite load_poly_composite(const params::PolyComposite& params)
	{
		rendering::PolyComposite composite;

		composite.set_local() = params.local;

		if (params.method)
		{
			const auto& method = params.method.value();
			switch (method.index())
			{
			case params::PolyComposite::MethodIndex::NGON:
			{
				const auto& m = std::get<params::PolyComposite::MethodIndex::NGON>(method);
				composite.set_composite() = { cmath::create_ngon(m.colors, m.points) };
				break;
			}
			case params::PolyComposite::MethodIndex::BORDERED_NGON:
			{
				const auto& m = std::get<params::PolyComposite::MethodIndex::BORDERED_NGON>(method);
				composite.set_composite() = cmath::create_bordered_ngon(m.ngon_base.fill_colors, m.ngon_base.border_colors, m.ngon_base.border_width, m.ngon_base.border_pivot, m.ngon_base.points);
				break;
			}
			case params::PolyComposite::MethodIndex::CONVEX_DECOMPOSITION:
			{
				const auto& m = std::get<params::PolyComposite::MethodIndex::CONVEX_DECOMPOSITION>(method);
				composite.set_composite() = cmath::Decompose{}(m.points);
				break;
			}
			}
		}

		return composite;
	}

	rendering::PolyComposite load_poly_composite(params::PolyComposite&& params)
	{
		rendering::PolyComposite composite;

		composite.set_local() = params.local;

		if (params.method)
		{
			auto& method = params.method.value();
			switch (method.index())
			{
			case params::PolyComposite::MethodIndex::NGON:
			{
				auto& m = std::get<params::PolyComposite::MethodIndex::NGON>(method);
				composite.set_composite() = { cmath::create_ngon(std::move(m.colors), std::move(m.points)) };
				break;
			}
			case params::PolyComposite::MethodIndex::BORDERED_NGON:
			{
				auto& m = std::get<params::PolyComposite::MethodIndex::BORDERED_NGON>(method);
				composite.set_composite() = cmath::create_bordered_ngon(std::move(m.ngon_base.fill_colors), std::move(m.ngon_base.border_colors),
					m.ngon_base.border_width, m.ngon_base.border_pivot, std::move(m.ngon_base.points));
				break;
			}
			case params::PolyComposite::MethodIndex::CONVEX_DECOMPOSITION:
			{
				auto& m = std::get<params::PolyComposite::MethodIndex::CONVEX_DECOMPOSITION>(method);
				composite.set_composite() = cmath::Decompose{}(m.points);
				break;
			}
			}
		}

		return composite;
	}

	rendering::NGon load_ngon(const TOMLNode& node)
	{
		if (LOG.enable.debug)
		{
			auto src = node["source"].value<std::string>();
			OLY_LOG_DEBUG(true, "REG") << LOG.source_info.full_source() << "Parsing ngon [" << (src ? *src : "") << "]." << LOG.nl;
		}

		params::NGon params;

		params.local = load_transform_2d(node["transform"]);

		auto toml_points = node["points"].as_array();
		if (toml_points)
		{
			size_t pt_idx = 0;
			for (auto& toml_point : *toml_points)
			{
				glm::vec2 pt;
				if (parse_vec((TOMLNode)toml_point, pt))
					params.ngon_base.points.push_back(pt);
				else
					OLY_LOG_WARNING(true, "REG") << LOG.source_info.full_source() << "Cannot convert ngon point #" << pt_idx << " to vec2." << LOG.nl;
				++pt_idx;
			}
		}

		auto toml_fill_colors = node["fill_colors"].as_array();
		if (toml_fill_colors)
		{
			size_t color_idx = 0;
			for (auto& toml_color : *toml_fill_colors)
			{
				glm::vec4 col;
				if (parse_vec((TOMLNode)toml_color, col))
					params.ngon_base.fill_colors.push_back(col);
				else
					OLY_LOG_WARNING(true, "REG") << LOG.source_info.full_source() << "Cannot convert ngon fill color #" << color_idx << " to vec4." << LOG.nl;
				++color_idx;
			}
		}

		auto toml_border_colors = node["border_colors"].as_array();
		if (toml_border_colors)
		{
			size_t color_idx = 0;
			for (auto& toml_color : *toml_border_colors)
			{
				glm::vec4 col;
				if (parse_vec((TOMLNode)toml_color, col))
					params.ngon_base.border_colors.push_back(col);
				else
					OLY_LOG_WARNING(true, "REG") << LOG.source_info.full_source() << "Cannot convert ngon border color #" << color_idx << " to vec4." << LOG.nl;
				++color_idx;
			}
		}

		parse_bool(node["bordered"], params.bordered);
		parse_float(node["border_width"], params.ngon_base.border_width);

		auto border_pivot = node["border_pivot"];
		if (auto str_border_pivot = border_pivot.value<std::string>())
		{
			const std::string& str = str_border_pivot.value();
			if (str == "outer")
				params.ngon_base.border_pivot = cmath::BorderPivot::OUTER;
			else if (str == "middle")
				params.ngon_base.border_pivot = cmath::BorderPivot::MIDDLE;
			else if (str == "inner")
				params.ngon_base.border_pivot = cmath::BorderPivot::INNER;
			else
				OLY_LOG_WARNING(true, "REG") << LOG.source_info.full_source() << "Unrecognized border pivot named value \"" << str << "\"." << LOG.nl;
		}
		else
			parse_float(border_pivot, params.ngon_base.border_pivot.v);

		if (LOG.enable.debug)
		{
			auto src = node["source"].value<std::string>();
			OLY_LOG_DEBUG(true, "REG") << LOG.source_info.full_source() << "Ngon [" << (src ? *src : "") << "] parsed." << LOG.nl;
		}

		return load_ngon(std::move(params));
	}

	rendering::NGon load_ngon(const params::NGon& params)
	{
		rendering::NGon ngon;

		ngon.set_local() = params.local;
		ngon.set_base() = params.ngon_base;
		ngon.set_bordered(params.bordered);

		return ngon;
	}

	rendering::NGon load_ngon(params::NGon&& params)
	{
		rendering::NGon ngon;

		ngon.set_local() = params.local;
		ngon.set_base() = std::move(params.ngon_base);
		ngon.set_bordered(params.bordered);

		return ngon;
	}
}
