#include "EllipseRegistry.h"

#include "Context.h"

namespace oly
{
	namespace rendering
	{
		void EllipseRegistry::load(const Context* context, const char* ellipse_registry_file)
		{
			auto toml = assets::load_toml(ellipse_registry_file);
			auto toml_registry = toml["ellipse_registry"];
			if (!toml_registry)
				return;
			auto ellipse_list = toml_registry["ellipse"].as_array();
			if (ellipse_list)
			{
				ellipse_list->for_each([this](auto&& node) {
					if constexpr (toml::is_table<decltype(node)>)
					{
						if (auto _name = node["name"].value<std::string>())
							ellipse_constructors[_name.value()] = node;
					}
					});
			}

			auto toml_auto_draw_list = toml_registry["auto draw list"].as_array();
			if (toml_auto_draw_list)
			{
				for (const auto& toml_ellipse : *toml_auto_draw_list)
				{
					auto _name = toml_ellipse.value<std::string>();
					if (!_name)
						continue;
					const std::string& name = _name.value();
					auto it = ellipse_constructors.find(name);
					if (it != ellipse_constructors.end())
						auto_draw_list.emplace(name, std::shared_ptr<Ellipse>(new Ellipse(create_ellipse(context, name))));
				}
			}
		}

		void EllipseRegistry::clear()
		{
			ellipse_constructors.clear();
			auto_draw_list.clear();
		}

		Ellipse EllipseRegistry::create_ellipse(const Context* context, const std::string& name) const
		{
			auto it = ellipse_constructors.find(name);
			if (it == ellipse_constructors.end())
				throw Error(ErrorCode::UNREGISTERED_ELLIPSE);
			const auto& node = it->second;

			Ellipse ellipse = context->ellipse();

			ellipse.transformer.local = assets::load_transform_2d(node, "transform");
			ellipse.post_set();

			auto& color = ellipse.ellipse.set_color();
			auto& dimension = ellipse.ellipse.set_dimension();
			assets::parse_vec4(node, "border inner color", color.border_inner);
			assets::parse_vec4(node, "border outer color", color.border_outer);
			assets::parse_vec4(node, "fill inner color", color.fill_inner);
			assets::parse_vec4(node, "fill outer color", color.fill_outer);
			assets::parse_float(node, "border", dimension.border);
			assets::parse_float(node, "border exp", dimension.border_exp);
			assets::parse_float(node, "fill exp", dimension.fill_exp);
			assets::parse_float(node, "width", dimension.width);
			assets::parse_float(node, "height", dimension.height);

			return ellipse;
		}

		std::weak_ptr<Ellipse> EllipseRegistry::ref_ellipse(const std::string& name) const
		{
			auto it = auto_draw_list.find(name);
			if (it == auto_draw_list.end())
				throw Error(ErrorCode::UNREGISTERED_ELLIPSE);
			return it->second;
		}
	}
}
