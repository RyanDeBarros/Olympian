#pragma once

#include "Ellipses.h"

#include "../Loader.h"

namespace oly
{
	class Context;
	namespace rendering
	{
		class EllipseRegistry
		{
			std::unordered_map<std::string, toml::table> ellipse_constructors;

			std::unordered_map<std::string, std::shared_ptr<Ellipse>> auto_loaded;

		public:
			void load(const Context& context, const char* ellipse_registry_file);
			void load(const Context& context, const std::string& ellipse_registry_file) { load(context, ellipse_registry_file.c_str()); }
			void clear();

			Ellipse create_ellipse(const Context& context, const std::string& name) const;
			std::weak_ptr<Ellipse> ref_ellipse(const std::string& name) const;
			void delete_ellipse(const Context& context, const std::string& name);
		};
	}
}
