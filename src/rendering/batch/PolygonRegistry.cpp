#include "PolygonRegistry.h"

#include "../../Olympian.h"

namespace oly
{
	namespace rendering
	{
		void PolygonRegistry::load(const Context* context, const char* polygon_registry_file)
		{
			auto toml = assets::load_toml(polygon_registry_file);
			auto toml_registry = toml["polygon_registry"];
			if (!toml_registry)
				return;
			auto polygon_list = toml_registry["polygon"].as_array();
			if (polygon_list)
			{
				polygon_list->for_each([this](auto&& node) {
					if constexpr (toml::is_table<decltype(node)>)
					{
						if (auto _name = node["name"].value<std::string>())
							polygon_constructors[_name.value()] = node;
					}
					});
			}
			auto composite_list = toml_registry["composite"].as_array();
			if (composite_list)
			{
				composite_list->for_each([this](auto&& node) {
					if constexpr (toml::is_table<decltype(node)>)
					{
						if (auto _name = node["name"].value<std::string>())
							composite_constructors[_name.value()] = node;
					}
					});
			}
			auto ngon_list = toml_registry["ngon"].as_array();
			if (ngon_list)
			{
				ngon_list->for_each([this](auto&& node) {
					if constexpr (toml::is_table<decltype(node)>)
					{
						if (auto _name = node["name"].value<std::string>())
							ngon_constructors[_name.value()] = node;
					}
					});
			}

			auto toml_auto_draw_list = toml_registry["auto draw list"].as_array();
			if (toml_auto_draw_list)
			{
				for (const auto& polygonal : *toml_auto_draw_list)
				{
					auto _name = polygonal.value<std::string>();
					if (!_name)
						continue;
					const std::string& name = _name.value();
					{
						auto it = polygon_constructors.find(name);
						if (it != polygon_constructors.end())
						{
							std::shared_ptr<Polygonal> poly(new Polygon(create_polygon(context, name)));
							if (!poly->initialized())
								poly->init();
							auto_draw_list.emplace(name, std::move(poly));
							continue;
						}
					}
					{
						auto it = composite_constructors.find(name);
						if (it != composite_constructors.end())
						{
							std::shared_ptr<Polygonal> poly(new Composite(create_composite(context, name)));
							if (!poly->initialized())
								poly->init();
							auto_draw_list.emplace(name, std::move(poly));
							continue;
						}
					}
					{
						auto it = ngon_constructors.find(name);
						if (it != ngon_constructors.end())
						{
							std::shared_ptr<Polygonal> poly(new NGon(create_ngon(context, name)));
							if (!poly->initialized())
								poly->init();
							auto_draw_list.emplace(name, std::move(poly));
							continue;
						}
					}
				}
			}
		}

		void PolygonRegistry::clear()
		{
			polygon_constructors.clear();
			composite_constructors.clear();
			ngon_constructors.clear();
			auto_draw_list.clear();
		}

		Polygon PolygonRegistry::create_polygon(const Context* context, const std::string& name) const
		{
			auto it = polygon_constructors.find(name);
			if (it == polygon_constructors.end())
				throw Error(ErrorCode::UNREGISTERED_POLYGON);
			const auto& node = it->second;

			Polygon polygon = context->polygon();

			polygon.transformer.local = assets::load_transform_2d(node, "transform");
			polygon.post_set();

			auto toml_points = node["points"].as_array();
			if (toml_points)
			{
				for (const auto& toml_point : *toml_points)
				{
					glm::vec2 pt;
					if (assets::parse_vec2(toml_point.as_array(), pt))
						polygon.polygon.points.push_back(pt);
				}
			}

			auto toml_colors = node["colors"].as_array();
			if (toml_colors)
			{
				for (const auto& toml_color : *toml_colors)
				{
					glm::vec4 col;
					if (assets::parse_vec4(toml_color.as_array(), col))
						polygon.polygon.colors.push_back(col);
				}
			}

			bool init = node["init"].value<bool>().value_or(false);
			if (init)
				polygon.init((GLushort)node["min range"].value<int64_t>().value_or(0), (GLushort)node["max range"].value<int64_t>().value_or(0));

			return polygon;
		}

		Composite PolygonRegistry::create_composite(const Context* context, const std::string& name) const
		{
			auto it = composite_constructors.find(name);
			if (it == composite_constructors.end())
				throw Error(ErrorCode::UNREGISTERED_POLYGON);
			const auto& node = it->second;

			Composite composite = context->composite();

			composite.transformer.local = assets::load_transform_2d(node, "transform");
			composite.post_set();

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
							if (assets::parse_vec2(toml_point.as_array(), pt))
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
							if (assets::parse_vec4(toml_color.as_array(), col))
								colors.push_back(col);
						}
					}

					composite.composite = { math::create_ngon(std::move(colors), std::move(points)) };
				}
				else if (method == "bordered ngon")
				{
					math::NGonBase ngon;

					auto toml_points = node["points"].as_array();
					if (toml_points)
					{
						for (const auto& toml_point : *toml_points)
						{
							glm::vec2 pt;
							if (assets::parse_vec2(toml_point.as_array(), pt))
								ngon.points.push_back(pt);
						}
					}

					auto toml_fill_colors = node["fill colors"].as_array();
					if (toml_fill_colors)
					{
						for (const auto& toml_color : *toml_fill_colors)
						{
							glm::vec4 col;
							if (assets::parse_vec4(toml_color.as_array(), col))
								ngon.fill_colors.push_back(col);
						}
					}

					auto toml_border_colors = node["border colors"].as_array();
					if (toml_border_colors)
					{
						for (const auto& toml_color : *toml_border_colors)
						{
							glm::vec4 col;
							if (assets::parse_vec4(toml_color.as_array(), col))
								ngon.border_colors.push_back(col);
						}
					}

					ngon.border_width = (float)node["border width"].value<double>().value_or(0.0);

					auto border_pivot = node["border pivot"];
					if (auto str_border_pivot = border_pivot.value<std::string>())
					{
						const std::string& str = str_border_pivot.value();
						if (str == "outer")
							ngon.border_pivot = math::BorderPivot::OUTER;
						else if (str == "middle")
							ngon.border_pivot = math::BorderPivot::MIDDLE;
						else if (str == "inner")
							ngon.border_pivot = math::BorderPivot::INNER;
					}
					else if (auto flt_border_pivot = border_pivot.value<double>())
						ngon.border_pivot = (float)flt_border_pivot.value();

					composite.composite = math::create_bordered_ngon(std::move(ngon.fill_colors), std::move(ngon.border_colors), ngon.border_width, ngon.border_pivot, std::move(ngon.points));
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
							if (assets::parse_vec2(toml_point.as_array(), pt))
								points.push_back(pt);
						}
					}
					composite.composite = math::composite_convex_decomposition(points);
				}
			}

			bool init = node["init"].value<bool>().value_or(false);
			if (init)
				composite.init((GLushort)node["min range"].value<int64_t>().value_or(0), (GLushort)node["max range"].value<int64_t>().value_or(0));

			return composite;
		}

		NGon PolygonRegistry::create_ngon(const Context* context, const std::string& name) const
		{
			auto it = ngon_constructors.find(name);
			if (it == ngon_constructors.end())
				throw Error(ErrorCode::UNREGISTERED_POLYGON);
			const auto& node = it->second;

			NGon ngon = context->ngon();

			ngon.transformer.local = assets::load_transform_2d(node, "transform");
			ngon.post_set();

			auto toml_points = node["points"].as_array();
			if (toml_points)
			{
				for (const auto& toml_point : *toml_points)
				{
					glm::vec2 pt;
					if (assets::parse_vec2(toml_point.as_array(), pt))
						ngon.base.points.push_back(pt);
				}
			}

			auto toml_fill_colors = node["fill colors"].as_array();
			if (toml_fill_colors)
			{
				for (const auto& toml_color : *toml_fill_colors)
				{
					glm::vec4 col;
					if (assets::parse_vec4(toml_color.as_array(), col))
						ngon.base.fill_colors.push_back(col);
				}
			}

			auto toml_border_colors = node["border colors"].as_array();
			if (toml_border_colors)
			{
				for (const auto& toml_color : *toml_border_colors)
				{
					glm::vec4 col;
					if (assets::parse_vec4(toml_color.as_array(), col))
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
					ngon.base.border_pivot = math::BorderPivot::OUTER;
				else if (str == "middle")
					ngon.base.border_pivot = math::BorderPivot::MIDDLE;
				else if (str == "inner")
					ngon.base.border_pivot = math::BorderPivot::INNER;
			}
			else if (auto flt_border_pivot = border_pivot.value<double>())
				ngon.base.border_pivot = (float)flt_border_pivot.value();

			bool init = node["init"].value<bool>().value_or(false);
			if (init)
				ngon.init((GLushort)node["min range"].value<int64_t>().value_or(0), (GLushort)node["max range"].value<int64_t>().value_or(0));

			return ngon;
		}

		std::weak_ptr<Polygonal> PolygonRegistry::ref_polygonal(const std::string& name) const
		{
			auto it = auto_draw_list.find(name);
			if (it == auto_draw_list.end())
				throw Error(ErrorCode::UNREGISTERED_POLYGON);
			return it->second;
		}
	}
}
