#include "Polygons.h"

#include "core/cmath/Triangulation.h"
#include "registries/Loader.h"

namespace oly::reg
{
	rendering::Polygon load_polygon(const TOMLNode& node)
	{
		params::Polygon params;

		params.local = load_transform_2d(node, "transform");

		auto toml_points = node["points"].as_array();
		if (toml_points)
		{
			size_t pt_idx = 0;
			for (const auto& toml_point : *toml_points)
			{
				glm::vec2 pt;
				if (parse_vec(toml_point.as_array(), pt))
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
			for (const auto& toml_color : *toml_colors)
			{
				glm::vec4 col;
				if (parse_vec(toml_color.as_array(), col))
					params.colors.push_back(col);
				else
					OLY_LOG_WARNING(true, "REG") << LOG.source_info.full_source() << "Cannot convert polygon point color #" << color_idx << " to vec4." << LOG.nl;
				++color_idx;
			}
		}

		return load_polygon(std::move(params));
	}

	rendering::Polygon load_polygon(const params::Polygon& params)
	{
		rendering::Polygon polygon;

		polygon.set_local() = params.local;
		polygon.polygon.points = params.points;
		polygon.polygon.colors = params.colors;

		polygon.init();
		return polygon;
	}

	rendering::Polygon load_polygon(params::Polygon&& params)
	{
		rendering::Polygon polygon;

		polygon.set_local() = params.local;
		polygon.polygon.points = std::move(params.points);
		polygon.polygon.colors = std::move(params.colors);

		polygon.init();
		return polygon;
	}

	rendering::PolyComposite load_poly_composite(const TOMLNode& node)
	{
		params::PolyComposite params;

		params.local = load_transform_2d(node, "transform");

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
					for (const auto& toml_point : *toml_points)
					{
						glm::vec2 pt;
						if (parse_vec(toml_point.as_array(), pt))
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
					for (const auto& toml_color : *toml_fill_colors)
					{
						glm::vec4 col;
						if (parse_vec(toml_color.as_array(), col))
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
					for (const auto& toml_point : *toml_points)
					{
						glm::vec2 pt;
						if (parse_vec(toml_point.as_array(), pt))
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
					for (const auto& toml_color : *toml_fill_colors)
					{
						glm::vec4 col;
						if (parse_vec(toml_color.as_array(), col))
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
					for (const auto& toml_color : *toml_border_colors)
					{
						glm::vec4 col;
						if (parse_vec(toml_color.as_array(), col))
							method.ngon_base.border_colors.push_back(col);
						else
							OLY_LOG_WARNING(true, "REG") << LOG.source_info.full_source() << "Cannot convert poly composite border color #" << color_idx << " to vec4." << LOG.nl;
						++color_idx;
					}
				}

				method.ngon_base.border_width = (float)node["border_width"].value<double>().value_or(0.0);

				auto border_pivot = node["border_pivot"];
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
				else if (auto flt_border_pivot = border_pivot.value<double>())
					method.ngon_base.border_pivot = (float)flt_border_pivot.value();

				params.method = std::move(method);
			}
			else if (method == "convex_decomposition")
			{
				params::PolyComposite::ConvexDecompositionMethod method;

				auto toml_points = node["points"].as_array();
				if (toml_points)
				{
					size_t pt_idx = 0;
					for (const auto& toml_point : *toml_points)
					{
						glm::vec2 pt;
						if (parse_vec(toml_point.as_array(), pt))
							method.points.push_back(pt);
						else
							OLY_LOG_WARNING(true, "REG") << LOG.source_info.full_source() << "Cannot convert poly composite point #" << pt_idx << " to vec2." << LOG.nl;
						++pt_idx;
					}
				}

				params.method = std::move(method);
			}
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
				composite.composite = { cmath::create_ngon(m.colors, m.points) };
				break;
			}
			case params::PolyComposite::MethodIndex::BORDERED_NGON:
			{
				const auto& m = std::get<params::PolyComposite::MethodIndex::BORDERED_NGON>(method);
				composite.composite = cmath::create_bordered_ngon(m.ngon_base.fill_colors, m.ngon_base.border_colors, m.ngon_base.border_width, m.ngon_base.border_pivot, m.ngon_base.points);
				break;
			}
			case params::PolyComposite::MethodIndex::CONVEX_DECOMPOSITION:
			{
				const auto& m = std::get<params::PolyComposite::MethodIndex::CONVEX_DECOMPOSITION>(method);
				composite.composite = cmath::Decompose{}(m.points);
				break;
			}
			}
		}

		composite.init();
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
				composite.composite = { cmath::create_ngon(std::move(m.colors), std::move(m.points)) };
				break;
			}
			case params::PolyComposite::MethodIndex::BORDERED_NGON:
			{
				auto& m = std::get<params::PolyComposite::MethodIndex::BORDERED_NGON>(method);
				composite.composite = cmath::create_bordered_ngon(std::move(m.ngon_base.fill_colors), std::move(m.ngon_base.border_colors),
					m.ngon_base.border_width, m.ngon_base.border_pivot, std::move(m.ngon_base.points));
				break;
			}
			case params::PolyComposite::MethodIndex::CONVEX_DECOMPOSITION:
			{
				auto& m = std::get<params::PolyComposite::MethodIndex::CONVEX_DECOMPOSITION>(method);
				composite.composite = cmath::Decompose{}(m.points);
				break;
			}
			}
		}

		composite.init();
		return composite;
	}

	rendering::NGon load_ngon(const TOMLNode& node)
	{
		params::NGon params;

		params.local = load_transform_2d(node, "transform");

		auto toml_points = node["points"].as_array();
		if (toml_points)
		{
			size_t pt_idx = 0;
			for (const auto& toml_point : *toml_points)
			{
				glm::vec2 pt;
				if (parse_vec(toml_point.as_array(), pt))
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
			for (const auto& toml_color : *toml_fill_colors)
			{
				glm::vec4 col;
				if (parse_vec(toml_color.as_array(), col))
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
			for (const auto& toml_color : *toml_border_colors)
			{
				glm::vec4 col;
				if (parse_vec(toml_color.as_array(), col))
					params.ngon_base.border_colors.push_back(col);
				else
					OLY_LOG_WARNING(true, "REG") << LOG.source_info.full_source() << "Cannot convert ngon border color #" << color_idx << " to vec4." << LOG.nl;
				++color_idx;
			}
		}

		params.bordered = node["bordered"].value<bool>().value_or(false);
		params.ngon_base.border_width = (float)node["border_width"].value<double>().value_or(0.0);

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
		else if (auto flt_border_pivot = border_pivot.value<double>())
			params.ngon_base.border_pivot = (float)flt_border_pivot.value();

		return load_ngon(std::move(params));
	}

	rendering::NGon load_ngon(const params::NGon& params)
	{
		rendering::NGon ngon;

		ngon.set_local() = params.local;
		ngon.base = params.ngon_base;
		ngon.bordered = params.bordered;

		ngon.init();
		return ngon;
	}

	rendering::NGon load_ngon(params::NGon&& params)
	{
		rendering::NGon ngon;

		ngon.set_local() = params.local;
		ngon.base = std::move(params.ngon_base);
		ngon.bordered = params.bordered;

		ngon.init();
		return ngon;
	}
}
