#include "IO.h"

#include <filesystem>
#include <fstream>
#include <sstream>
#include <iostream>

#include "core/util/LoggerOperators.h"
#include "core/base/Errors.h"

namespace oly::io
{
	std::vector<std::string> read_file_lines(const char* filepath)
	{
		std::ifstream file(filepath);
		if (!file)
		{
			OLY_LOG_ERROR(true) << LOG.source_info.full_source() << "Could not open \"" << filepath << "\" for reading" << LOG.nl;
			throw Error(ErrorCode::FILE_IO);
		}

		std::vector<std::string> lines;
		std::string line;
		while (std::getline(file, line))
			lines.push_back(std::move(line));
		return lines;
	}

	std::vector<std::string> read_file_lines(const ResourcePath& filepath)
	{
		std::ifstream file = filepath.get_ifstream();
		if (!file)
		{
			OLY_LOG_ERROR(true) << LOG.source_info.full_source() << "Could not open " << filepath << " for reading" << LOG.nl;
			throw Error(ErrorCode::FILE_IO);
		}

		std::vector<std::string> lines;
		std::string line;
		while (std::getline(file, line))
			lines.push_back(std::move(line));
		return lines;
	}

	std::string read_file(const char* filepath)
	{
		std::ifstream file(filepath);
		if (!file)
		{
			OLY_LOG_ERROR(true) << LOG.source_info.full_source() << "Could not open \"" << filepath << "\" for reading" << LOG.nl;
			throw Error(ErrorCode::FILE_IO);
		}
		
		std::ostringstream oss;
		oss << file.rdbuf();
		return oss.str();
	}

	std::string read_file(const ResourcePath& filepath)
	{
		std::ifstream file = filepath.get_ifstream();
		if (!file)
		{
			OLY_LOG_ERROR(true) << LOG.source_info.full_source() << "Could not open " << filepath << " for reading" << LOG.nl;
			throw Error(ErrorCode::FILE_IO);
		}

		std::ostringstream oss;
		oss << file.rdbuf();
		return oss.str();
	}

	std::vector<unsigned char> read_file_uc(const ResourcePath& filepath)
	{
		
		std::ifstream file = filepath.get_ifstream(std::ios::binary | std::ios::ate);
		if (!file)
		{
			OLY_LOG_ERROR(true) << LOG.source_info.full_source() << "Could not open " << filepath << " for reading" << LOG.nl;
			throw Error(ErrorCode::FILE_IO);
		}

		std::streamsize size = file.tellg();
		file.seekg(0, std::ios::beg);

		std::vector<unsigned char> content(size);
		if (!file.read(reinterpret_cast<char*>(content.data()), size))
		{
			OLY_LOG_ERROR(true) << LOG.source_info.full_source() << "Failed to read " << filepath << LOG.nl;
			throw Error(ErrorCode::FILE_IO);
		}

		return content;
	}

	static std::string read_template_content(std::string&& content, const std::unordered_map<std::string, std::string>& tmpl)
	{
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

	std::string read_template_file(const char* file, const std::unordered_map<std::string, std::string>& tmpl)
	{
		return read_template_content(read_file(file), tmpl);
	}

	std::string read_template_file(const ResourcePath& file, const std::unordered_map<std::string, std::string>& tmpl)
	{
		return read_template_content(read_file(file), tmpl);
	}
}
