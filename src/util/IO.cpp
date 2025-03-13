#include "IO.h"

#include "Errors.h"

#include <filesystem>
#include <fstream>
#include <sstream>
#include <iostream>

std::string oly::io::read_file(const char* filepath)
{
	std::ifstream file(filepath, std::ios_base::in);
	if (!file)
		throw Error(ErrorCode::FILE_IO, "Could not open \"" + std::filesystem::absolute(std::filesystem::path(filepath)).string() + "\" for reading");
	std::ostringstream oss;
	oss << file.rdbuf();
	return oss.str();
}

std::vector<unsigned char> oly::io::read_file_uc(const char* filepath)
{
	FILE* file = nullptr;
	if (fopen_s(&file, filepath, "r") != 0 || !file)
		throw Error(ErrorCode::FILE_IO, "Could not open \"" + std::filesystem::absolute(std::filesystem::path(filepath)).string() + "\" for reading");
	std::vector<unsigned char> content;
	fseek(file, 0, SEEK_END);
	content.resize(ftell(file));
	fseek(file, 0, SEEK_SET);
	fread(content.data(), content.size(), 1, file);
	fclose(file);
	return content;
}

std::string oly::io::read_template_file(const char* filepath, const std::unordered_map<std::string, std::string>& tmpl)
{
	std::string content = read_file(filepath);
	for (const auto& [placeholder, value] : tmpl)
	{
		size_t pos = 0;
		while ((pos = content.find(placeholder, pos)) != std::string::npos)
		{
			content.replace(pos, placeholder.length(), value);
			pos += value.length();
		}
	}
	return content;
}
