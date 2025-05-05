#include "EllipseRegistry.h"

#include "core/base/Context.h"
#include "registries/Loader.h"

namespace oly::reg
{
	void EllipseRegistry::load(const char* ellipse_registry_file)
	{
		auto toml = load_toml(ellipse_registry_file);
		auto ellipse_list = toml["ellipse"].as_array();
		if (!ellipse_list)
			return;
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
							auto_loaded.emplace(name, move_shared(create_ellipse(name)));
					}
				}
			}
			});
	}

	void EllipseRegistry::clear()
	{
		ellipse_constructors.clear();
		auto_loaded.clear();
	}

	rendering::Ellipse EllipseRegistry::create_ellipse(const std::string& name) const
	{
		auto it = ellipse_constructors.find(name);
		if (it == ellipse_constructors.end())
			throw Error(ErrorCode::UNREGISTERED_ELLIPSE);
		const auto& node = it->second;

		rendering::Ellipse ellipse = context::ellipse();

		ellipse.set_local() = load_transform_2d(node, "transform");

		auto& color = ellipse.ellipse.set_color();
		auto& dimension = ellipse.ellipse.set_dimension();
		parse_vec4(node, "border inner color", color.border_inner);
		parse_vec4(node, "border outer color", color.border_outer);
		parse_vec4(node, "fill inner color", color.fill_inner);
		parse_vec4(node, "fill outer color", color.fill_outer);
		parse_float(node, "border", dimension.border);
		parse_float(node, "border exp", dimension.border_exp);
		parse_float(node, "fill exp", dimension.fill_exp);
		parse_float(node, "width", dimension.width);
		parse_float(node, "height", dimension.height);

		return ellipse;
	}

	std::weak_ptr<rendering::Ellipse> EllipseRegistry::ref_ellipse(const std::string& name) const
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
