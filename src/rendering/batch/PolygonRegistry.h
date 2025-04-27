#pragma once

#include "Polygons.h"

#include "../Loader.h"

namespace oly
{
	class Context;
	namespace rendering
	{
		class PolygonRegistry
		{
			std::unordered_map<std::string, toml::table> polygon_constructors;
			std::unordered_map<std::string, toml::table> composite_constructors;
			std::unordered_map<std::string, toml::table> ngon_constructors;

			std::unordered_map<std::string, std::shared_ptr<Polygonal>> auto_loaded;

		public:
			void load(const Context* context, const char* polygon_registry_file);
			void load(const Context* context, const std::string& polygon_registry_file) { load(context, polygon_registry_file.c_str()); }
			void clear();

			Polygon create_polygon(const Context* context, const std::string& name) const;
			Composite create_composite(const Context* context, const std::string& name) const;
			NGon create_ngon(const Context* context, const std::string& name) const;
			std::weak_ptr<Polygonal> ref_polygonal(const std::string& name) const;
			void delete_polygonal(const Context* context, const std::string& name);
		};
	}
}
