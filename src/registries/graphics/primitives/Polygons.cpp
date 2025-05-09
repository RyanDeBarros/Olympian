#include "Polygons.h"

#include "core/base/Context.h"
#include "core/math/Triangulation.h"
#include "registries/Loader.h"

namespace oly::reg
{
	rendering::Polygon load_polygon(const TOMLNode& node)
	{
		rendering::Polygon polygon = context::polygon();

		polygon.set_local() = load_transform_2d(node, "transform");

		auto toml_points = node["points"].as_array();
		if (toml_points)
		{
			for (const auto& toml_point : *toml_points)
			{
				glm::vec2 pt;
				if (parse_vec2(toml_point.as_array(), pt))
					polygon.polygon.points.push_back(pt);
			}
		}

		auto toml_colors = node["colors"].as_array();
		if (toml_colors)
		{
			for (const auto& toml_color : *toml_colors)
			{
				glm::vec4 col;
				if (parse_vec4(toml_color.as_array(), col))
					polygon.polygon.colors.push_back(col);
			}
		}

		polygon.init();
		return polygon;
	}

	rendering::PolyComposite load_poly_composite(const TOMLNode& node)
	{
		rendering::PolyComposite composite = context::poly_composite();

		composite.set_local() = load_transform_2d(node, "transform");

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
					for (const auto& toml_point : *toml_points)
					{
						glm::vec2 pt;
						if (parse_vec2(toml_point.as_array(), pt))
							points.push_back(pt);
					}
				}

				std::vector<glm::vec4> colors;
				auto toml_fill_colors = node["colors"].as_array();
				if (toml_fill_colors)
				{
					for (const auto& toml_color : *toml_fill_colors)
					{
						glm::vec4 col;
						if (parse_vec4(toml_color.as_array(), col))
							colors.push_back(col);
					}
				}

				composite.composite = { cmath::create_ngon(std::move(colors), std::move(points)) };
			}
			else if (method == "bordered ngon")
			{
				cmath::NGonBase ngon;

				auto toml_points = node["points"].as_array();
				if (toml_points)
				{
					for (const auto& toml_point : *toml_points)
					{
						glm::vec2 pt;
						if (parse_vec2(toml_point.as_array(), pt))
							ngon.points.push_back(pt);
					}
				}

				auto toml_fill_colors = node["fill colors"].as_array();
				if (toml_fill_colors)
				{
					for (const auto& toml_color : *toml_fill_colors)
					{
						glm::vec4 col;
						if (parse_vec4(toml_color.as_array(), col))
							ngon.fill_colors.push_back(col);
					}
				}

				auto toml_border_colors = node["border colors"].as_array();
				if (toml_border_colors)
				{
					for (const auto& toml_color : *toml_border_colors)
					{
						glm::vec4 col;
						if (parse_vec4(toml_color.as_array(), col))
							ngon.border_colors.push_back(col);
					}
				}

				ngon.border_width = (float)node["border width"].value<double>().value_or(0.0);

				auto border_pivot = node["border pivot"];
				if (auto str_border_pivot = border_pivot.value<std::string>())
				{
					const std::string& str = str_border_pivot.value();
					if (str == "outer")
						ngon.border_pivot = cmath::BorderPivot::OUTER;
					else if (str == "middle")
						ngon.border_pivot = cmath::BorderPivot::MIDDLE;
					else if (str == "inner")
						ngon.border_pivot = cmath::BorderPivot::INNER;
				}
				else if (auto flt_border_pivot = border_pivot.value<double>())
					ngon.border_pivot = (float)flt_border_pivot.value();

				composite.composite = cmath::create_bordered_ngon(std::move(ngon.fill_colors), std::move(ngon.border_colors), ngon.border_width, ngon.border_pivot, std::move(ngon.points));
			}
			else if (method == "convex decomposition")
			{
				std::vector<glm::vec2> points;
				auto toml_points = node["points"].as_array();
				if (toml_points)
				{
					for (const auto& toml_point : *toml_points)
					{
						glm::vec2 pt;
						if (parse_vec2(toml_point.as_array(), pt))
							points.push_back(pt);
					}
				}
				composite.composite = cmath::composite_convex_decomposition(points);
			}
		}

		composite.init();
		return composite;
	}

	rendering::NGon load_ngon(const TOMLNode& node)
	{
		rendering::NGon ngon = context::ngon();

		ngon.set_local() = load_transform_2d(node, "transform");

		auto toml_points = node["points"].as_array();
		if (toml_points)
		{
			for (const auto& toml_point : *toml_points)
			{
				glm::vec2 pt;
				if (parse_vec2(toml_point.as_array(), pt))
					ngon.base.points.push_back(pt);
			}
		}

		auto toml_fill_colors = node["fill colors"].as_array();
		if (toml_fill_colors)
		{
			for (const auto& toml_color : *toml_fill_colors)
			{
				glm::vec4 col;
				if (parse_vec4(toml_color.as_array(), col))
					ngon.base.fill_colors.push_back(col);
			}
		}

		auto toml_border_colors = node["border colors"].as_array();
		if (toml_border_colors)
		{
			for (const auto& toml_color : *toml_border_colors)
			{
				glm::vec4 col;
				if (parse_vec4(toml_color.as_array(), col))
					ngon.base.border_colors.push_back(col);
			}
		}

		ngon.bordered = node["bordered"].value<bool>().value_or(false);
		ngon.base.border_width = (float)node["border width"].value<double>().value_or(0.0);

		auto border_pivot = node["border pivot"];
		if (auto str_border_pivot = border_pivot.value<std::string>())
		{
			const std::string& str = str_border_pivot.value();
			if (str == "outer")
				ngon.base.border_pivot = cmath::BorderPivot::OUTER;
			else if (str == "middle")
				ngon.base.border_pivot = cmath::BorderPivot::MIDDLE;
			else if (str == "inner")
				ngon.base.border_pivot = cmath::BorderPivot::INNER;
		}
		else if (auto flt_border_pivot = border_pivot.value<double>())
			ngon.base.border_pivot = (float)flt_border_pivot.value();

		ngon.init();
		return ngon;
	}
}
