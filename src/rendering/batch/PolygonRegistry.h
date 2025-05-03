#pragma once

#include "Polygons.h"

#include "../Loader.h"

namespace oly
{
	namespace rendering
	{
		class PolygonRegistry
		{
			std::unordered_map<std::string, toml::table> polygon_constructors;
			std::unordered_map<std::string, toml::table> composite_constructors;
			std::unordered_map<std::string, toml::table> ngon_constructors;

			std::unordered_map<std::string, std::shared_ptr<Polygonal>> auto_loaded;

		public:
			void load(const char* polygon_registry_file);
			void load(const std::string& polygon_registry_file) { load(polygon_registry_file.c_str()); }
			void clear();

			Polygon create_polygon(const std::string& name) const;
			Composite create_composite(const std::string& name) const;
			NGon create_ngon(const std::string& name) const;
			std::weak_ptr<Polygonal> ref_polygonal(const std::string& name) const;
			void delete_polygonal(const std::string& name);
		};
	}
}
