#pragma once

#include "external/TOML.h"

#include "graphics/primitives/Ellipses.h"

namespace oly::reg
{
	class EllipseRegistry
	{
		std::unordered_map<std::string, toml::table> ellipse_constructors;
		std::unordered_map<std::string, std::shared_ptr<rendering::Ellipse>> auto_loaded;

	public:
		void load(const char* ellipse_registry_file);
		void load(const std::string& ellipse_registry_file) { load(ellipse_registry_file.c_str()); }
		void clear();

		rendering::Ellipse create_ellipse(const std::string& name) const;
		std::weak_ptr<rendering::Ellipse> ref_ellipse(const std::string& name) const;
		void delete_ellipse(const std::string& name);
	};
}
