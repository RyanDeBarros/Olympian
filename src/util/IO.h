#pragma once

#include <unordered_map>
#include <string>

namespace oly
{
	namespace io
	{
		extern std::string read_file(const char* filepath);
		extern std::vector<unsigned char> read_file_uc(const char* filepath);
		extern std::string read_template_file(const char* filepath, const std::unordered_map<std::string, std::string>& tmpl);
	}
}
