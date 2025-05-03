#include "EllipseRegistry.h"

#include "Context.h"

namespace oly
{
	namespace rendering
	{
		void EllipseRegistry::load(const char* ellipse_registry_file)
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
						{
							const std::string& name = _name.value();
							ellipse_constructors[name] = node;
							if (auto _init = node["init"].value<bool>())
							{
								if (_init.value())
									auto_loaded.emplace(name, std::shared_ptr<Ellipse>(new Ellipse(create_ellipse(name))));
							}
						}
					}
					});
			}
		}

		void EllipseRegistry::clear()
		{
			ellipse_constructors.clear();
			auto_loaded.clear();
		}

		Ellipse EllipseRegistry::create_ellipse(const std::string& name) const
		{
			auto it = ellipse_constructors.find(name);
			if (it == ellipse_constructors.end())
				throw Error(ErrorCode::UNREGISTERED_ELLIPSE);
			const auto& node = it->second;

			Ellipse ellipse = context::ellipse();

			ellipse.set_local() = assets::load_transform_2d(node, "transform");

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
			auto it = auto_loaded.find(name);
			if (it == auto_loaded.end())
				throw Error(ErrorCode::UNREGISTERED_ELLIPSE);
			return it->second;
		}

		void EllipseRegistry::delete_ellipse(const std::string& name)
		{
			auto_loaded.erase(name);
		}
	}
}
