#pragma once

#include <unordered_map>
#include <string>
#include <vector>

namespace oly::io
{
	extern std::vector<std::string> read_file_lines(const char* filepath);
	inline std::vector<std::string> read_file_lines(const std::string& filepath) { return read_file_lines(filepath.c_str()); }
	extern std::string read_file(const char* filepath);
	inline std::string read_file(const std::string& filepath) { return read_file(filepath.c_str()); }
	extern std::vector<unsigned char> read_file_uc(const char* filepath);
	inline std::vector<unsigned char> read_file_uc(const std::string& filepath) { return read_file_uc(filepath.c_str()); }
	extern std::string read_template_file(const char* filepath, const std::unordered_map<std::string, std::string>& tmpl);
	inline std::string read_template_file(const std::string& filepath, const std::unordered_map<std::string, std::string>& tmpl)
		{ return read_template_file(filepath.c_str(), tmpl); }
	extern std::string file_extension(const char* filepath);
	inline std::string file_extension(const std::string& filepath) { return file_extension(filepath.c_str()); }
	extern std::string directory_of(const char* filepath);
	inline std::string directory_of(const std::string& filepath) { return directory_of(filepath.c_str()); }
}
