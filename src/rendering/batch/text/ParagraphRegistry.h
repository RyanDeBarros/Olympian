#pragma once

#include "Paragraph.h"

#include "registries/Loader.h"

namespace oly
{
	namespace rendering
	{
		class ParagraphRegistry
		{
			std::unordered_map<std::string, toml::table> constructors;
			std::unordered_map<std::string, std::shared_ptr<Paragraph>> auto_loaded;

		public:
			void load(const char* registry_file);
			void load(const std::string& registry_file) { load(registry_file.c_str()); }
			void clear();

			Paragraph create_paragraph(const std::string& name) const;
			std::weak_ptr<Paragraph> ref_paragraph(const std::string& name) const;
			void delete_paragraph(const std::string& name);
		};
	}
}
