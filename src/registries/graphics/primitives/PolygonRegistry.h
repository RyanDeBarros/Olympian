#pragma once

#include "external/TOML.h"

#include "graphics/primitives/Polygons.h"

namespace oly::reg
{
	class PolygonRegistry
	{
		std::unordered_map<std::string, toml::table> polygon_constructors;
		std::unordered_map<std::string, toml::table> composite_constructors;
		std::unordered_map<std::string, toml::table> ngon_constructors;

		std::unordered_map<std::string, std::shared_ptr<rendering::Polygonal>> auto_loaded;

	public:
		void load(const char* polygon_registry_file);
		void load(const std::string& polygon_registry_file) { load(polygon_registry_file.c_str()); }
		void clear();

		rendering::Polygon create_polygon(const std::string& name) const;
		rendering::PolyComposite create_composite(const std::string& name) const;
		rendering::NGon create_ngon(const std::string& name) const;
		std::weak_ptr<rendering::Polygonal> ref_polygonal(const std::string& name) const;
		void delete_polygonal(const std::string& name);
	};
}
