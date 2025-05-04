#pragma once

#include "external/TOML.h"

#include "graphics/text/Paragraph.h"

namespace oly::reg
{
	class ParagraphRegistry
	{
		std::unordered_map<std::string, toml::table> constructors;
		std::unordered_map<std::string, std::shared_ptr<rendering::Paragraph>> auto_loaded;

	public:
		void load(const char* registry_file);
		void load(const std::string& registry_file) { load(registry_file.c_str()); }
		void clear();

		rendering::Paragraph create_paragraph(const std::string& name) const;
		std::weak_ptr<rendering::Paragraph> ref_paragraph(const std::string& name) const;
		void delete_paragraph(const std::string& name);
	};
}
