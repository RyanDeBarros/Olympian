#pragma once

#include <unordered_map>
#include <string>
#include <vector>

#include "core/util/ResourcePath.h"

namespace oly::io
{
	extern std::vector<std::string> read_file_lines(const ResourcePath& file);
	extern std::string read_file(const ResourcePath& file);
	extern std::vector<unsigned char> read_file_uc(const ResourcePath& file);
	extern std::string read_template_file(const ResourcePath& file, const std::unordered_map<std::string, std::string>& tmpl);
}
