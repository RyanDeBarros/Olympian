#include "Polygons.h"

#include "core/cmath/Triangulation.h"
#include "assets/Loader.h"

namespace oly::assets
{
	rendering::Polygon load_polygon(TOMLNode node, const char* source)
	{
		OLY_LOG_DEBUG(true, "ASSETS") << LOG.source_info.full_source() << "Parsing polygon [" << (source ? source : "") << "]..." << LOG.nl;

		rendering::Polygon polygon;

		polygon.transformer = load_transformer_2d(node["transformer"]);

		std::vector<glm::vec2> points;
		auto toml_points = node["points"].as_array();
		if (toml_points)
		{
			size_t pt_idx = 0;
			for (auto& toml_point : *toml_points)
			{
				glm::vec2 pt;
				if (parse_vec((TOMLNode)toml_point, pt))
					points.push_back(pt);
				else
					OLY_LOG_WARNING(true, "ASSETS") << LOG.source_info.full_source() << "Cannot convert polygon point #" << pt_idx << " to vec2." << LOG.nl;
				++pt_idx;
			}
		}
		polygon.set_points() = std::move(points);

		std::vector<glm::vec4> colors;
		auto toml_colors = node["colors"].as_array();
		if (toml_colors)
		{
			size_t color_idx = 0;
			for (auto& toml_color : *toml_colors)
			{
				glm::vec4 col;
				if (parse_vec((TOMLNode)toml_color, col))
					colors.push_back(col);
				else
					OLY_LOG_WARNING(true, "ASSETS") << LOG.source_info.full_source() << "Cannot convert polygon point color #" << color_idx << " to vec4." << LOG.nl;
				++color_idx;
			}
		}
		polygon.set_colors() = std::move(colors);

		OLY_LOG_DEBUG(true, "ASSETS") << LOG.source_info.full_source() << "...Polygon [" << (source ? source : "") << "] parsed." << LOG.nl;

		return polygon;
	}

	rendering::PolyComposite load_poly_composite(TOMLNode node, const char* source)
	{
		OLY_LOG_DEBUG(true, "ASSETS") << LOG.source_info.full_source() << "Parsing poly composite [" << (source ? source : "") << "]..." << LOG.nl;

		rendering::PolyComposite polygon;

		polygon.transformer = load_transformer_2d(node["transformer"]);

		auto toml_method = node["method"].value<std::string>();
		if (toml_method)
		{
			const std::string& method = toml_method.value();
			if (method == "ngon")
			{
				std::vector<glm::vec2> points;
				auto toml_points = node["points"].as_array();
				if (toml_points)
				{
					size_t pt_idx = 0;
					for (auto& toml_point : *toml_points)
					{
						glm::vec2 pt;
						if (parse_vec((TOMLNode)toml_point, pt))
							points.push_back(pt);
						else
							OLY_LOG_WARNING(true, "ASSETS") << LOG.source_info.full_source() << "Cannot convert poly composite point #" << pt_idx << " to vec2." << LOG.nl;
						++pt_idx;
					}
				}

				std::vector<glm::vec4> colors;
				auto toml_fill_colors = node["colors"].as_array();
				if (toml_fill_colors)
				{
					size_t color_idx = 0;
					for (auto& toml_color : *toml_fill_colors)
					{
						glm::vec4 col;
						if (parse_vec((TOMLNode)toml_color, col))
							colors.push_back(col);
						else
							OLY_LOG_WARNING(true, "ASSETS") << LOG.source_info.full_source() << "Cannot convert poly composite point color #" << color_idx << " to vec4." << LOG.nl;
						++color_idx;
					}
				}

				polygon.set_composite() = cmath::Polygon2DComposite{ cmath::create_ngon(std::move(colors), std::move(points)) };
			}
			else if (method == "bordered_ngon")
			{
				cmath::NGonBase ngon_base;

				auto toml_points = node["points"].as_array();
				if (toml_points)
				{
					size_t pt_idx = 0;
					for (auto& toml_point : *toml_points)
					{
						glm::vec2 pt;
						if (parse_vec((TOMLNode)toml_point, pt))
							ngon_base.points.push_back(pt);
						else
							OLY_LOG_WARNING(true, "ASSETS") << LOG.source_info.full_source() << "Cannot convert poly composite point #" << pt_idx << " to vec2." << LOG.nl;
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
							ngon_base.fill_colors.push_back(col);
						else
							OLY_LOG_WARNING(true, "ASSETS") << LOG.source_info.full_source() << "Cannot convert poly composite fill color #" << color_idx << " to vec4." << LOG.nl;
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
							ngon_base.border_colors.push_back(col);
						else
							OLY_LOG_WARNING(true, "ASSETS") << LOG.source_info.full_source() << "Cannot convert poly composite border color #" << color_idx << " to vec4." << LOG.nl;
						++color_idx;
					}
				}

				parse_float(node["border_width"], ngon_base.border_width);

				if (auto border_pivot = node["border_pivot"])
				{
					if (auto str_border_pivot = border_pivot.value<std::string>())
					{
						const std::string& str = str_border_pivot.value();
						if (str == "outer")
							ngon_base.border_pivot = cmath::BorderPivot::OUTER;
						else if (str == "middle")
							ngon_base.border_pivot = cmath::BorderPivot::MIDDLE;
						else if (str == "inner")
							ngon_base.border_pivot = cmath::BorderPivot::INNER;
						else
							OLY_LOG_WARNING(true, "ASSETS") << LOG.source_info.full_source() << "Unrecognized border pivot named value \"" << str << "\"." << LOG.nl;
					}
					else
						parse_float(border_pivot, ngon_base.border_pivot.v);
				}

				polygon.set_composite() = cmath::create_bordered_ngon(std::move(ngon_base.fill_colors), std::move(ngon_base.border_colors),
					ngon_base.border_width, ngon_base.border_pivot, std::move(ngon_base.points));
			}
			else if (method == "convex_decomposition")
			{
				std::vector<glm::vec2> points;

				auto toml_points = node["points"].as_array();
				if (toml_points)
				{
					size_t pt_idx = 0;
					for (auto& toml_point : *toml_points)
					{
						glm::vec2 pt;
						if (parse_vec((TOMLNode)toml_point, pt))
							points.push_back(pt);
						else
							OLY_LOG_WARNING(true, "ASSETS") << LOG.source_info.full_source() << "Cannot convert poly composite point #" << pt_idx << " to vec2." << LOG.nl;
						++pt_idx;
					}
				}

				polygon.set_composite() = cmath::Decompose{}(std::move(points));
			}
		}

		OLY_LOG_DEBUG(true, "ASSETS") << LOG.source_info.full_source() << "...Poly composite [" << (source ? source : "") << "] parsed." << LOG.nl;

		return polygon;
	}

	rendering::NGon load_ngon(TOMLNode node, const char* source)
	{
		OLY_LOG_DEBUG(true, "ASSETS") << LOG.source_info.full_source() << "Parsing ngon [" << (source ? source : "") << "]..." << LOG.nl;

		rendering::NGon polygon;

		polygon.transformer = load_transformer_2d(node["transformer"]);

		cmath::NGonBase ngon_base;

		auto toml_points = node["points"].as_array();
		if (toml_points)
		{
			size_t pt_idx = 0;
			for (auto& toml_point : *toml_points)
			{
				glm::vec2 pt;
				if (parse_vec((TOMLNode)toml_point, pt))
					ngon_base.points.push_back(pt);
				else
					OLY_LOG_WARNING(true, "ASSETS") << LOG.source_info.full_source() << "Cannot convert ngon point #" << pt_idx << " to vec2." << LOG.nl;
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
					ngon_base.fill_colors.push_back(col);
				else
					OLY_LOG_WARNING(true, "ASSETS") << LOG.source_info.full_source() << "Cannot convert ngon fill color #" << color_idx << " to vec4." << LOG.nl;
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
					ngon_base.border_colors.push_back(col);
				else
					OLY_LOG_WARNING(true, "ASSETS") << LOG.source_info.full_source() << "Cannot convert ngon border color #" << color_idx << " to vec4." << LOG.nl;
				++color_idx;
			}
		}

		bool bordered;
		if (parse_bool(node["bordered"], bordered))
			polygon.set_bordered(bordered);
		parse_float(node["border_width"], ngon_base.border_width);

		auto border_pivot = node["border_pivot"];
		if (auto str_border_pivot = border_pivot.value<std::string>())
		{
			const std::string& str = str_border_pivot.value();
			if (str == "outer")
				ngon_base.border_pivot = cmath::BorderPivot::OUTER;
			else if (str == "middle")
				ngon_base.border_pivot = cmath::BorderPivot::MIDDLE;
			else if (str == "inner")
				ngon_base.border_pivot = cmath::BorderPivot::INNER;
			else
				OLY_LOG_WARNING(true, "ASSETS") << LOG.source_info.full_source() << "Unrecognized border pivot named value \"" << str << "\"." << LOG.nl;
		}
		else
			parse_float(border_pivot, ngon_base.border_pivot.v);

		polygon.set_base() = std::move(ngon_base);

		OLY_LOG_DEBUG(true, "ASSETS") << LOG.source_info.full_source() << "...Ngon [" << (source ? source : "") << "] parsed." << LOG.nl;

		return polygon;
	}
}
