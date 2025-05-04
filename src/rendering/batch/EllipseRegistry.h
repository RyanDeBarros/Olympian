#pragma once

#include "Ellipses.h"

#include "registries/Loader.h"

namespace oly
{
	namespace rendering
	{
		class EllipseRegistry
		{
			std::unordered_map<std::string, toml::table> ellipse_constructors;
			std::unordered_map<std::string, std::shared_ptr<Ellipse>> auto_loaded;

		public:
			void load(const char* ellipse_registry_file);
			void load(const std::string& ellipse_registry_file) { load(ellipse_registry_file.c_str()); }
			void clear();

			Ellipse create_ellipse(const std::string& name) const;
			std::weak_ptr<Ellipse> ref_ellipse(const std::string& name) const;
			void delete_ellipse(const std::string& name);
		};
	}
}
