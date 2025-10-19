#pragma once

#include <unordered_map>
#include <string>
#include <vector>

#include "core/util/ResourcePath.h"

namespace oly::io
{
	extern std::vector<std::string> read_file_lines(const char* file);
	inline std::vector<std::string> read_file_lines(const std::string& file) { return read_file_lines(file.c_str()); }
	extern std::vector<std::string> read_file_lines(const ResourcePath& file);

	extern std::string read_file(const char* file);
	inline std::string read_file(const std::string& file) { return read_file(file.c_str()); }
	extern std::string read_file(const ResourcePath& file);

	extern std::vector<unsigned char> read_file_uc(const ResourcePath& file);

	extern std::string read_template_file(const char* file, const std::unordered_map<std::string, std::string>& tmpl);
	inline std::string read_template_file(const std::string& file, const std::unordered_map<std::string, std::string>& tmpl)
		{ return read_template_file(file.c_str(), tmpl); }
	extern std::string read_template_file(const ResourcePath& file, const std::unordered_map<std::string, std::string>& tmpl);
}
