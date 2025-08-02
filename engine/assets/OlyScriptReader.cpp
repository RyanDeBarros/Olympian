#include "OlyScriptReader.h"

#include <sstream>

#include "core/util/IO.h"
#include "core/util/Logger.h"
#include "core/algorithms/STLUtils.h"

namespace oly::assets
{
	static void remove_comments(std::string& line)
	{
		std::vector<std::pair<size_t, size_t>> remove_substrings;

		bool in_comment = false;
		size_t opening_pos = size_t(-1);
		bool in_string = false;
		bool escaping_quote = false;

		for (size_t i = 0; i < line.size(); ++i)
		{
			if (in_comment)
			{
				if (line[i] == '`')
				{
					remove_substrings.emplace_back(opening_pos, i + 1);
					opening_pos = size_t(-1);
					in_comment = false;
				}
			}
			else if (in_string)
			{
				if (line[i] == '\"')
				{
					if (escaping_quote)
						escaping_quote = false;
					else
						in_string = false;
				}
				else if (line[i] == '\\')
					escaping_quote = !escaping_quote;
				else
					escaping_quote = false;
			}
			else
			{
				if (line[i] == '`')
				{
					opening_pos = i;
					in_comment = true;
				}
				else if (line[i] == '\"')
				{
					in_string = true;
					escaping_quote = false;
				}
			}
		}

		// unclosed comment
		if (in_comment)
			line.erase(line.begin() + opening_pos, line.end());

		// closed comments
		size_t offset = 0;
		for (auto [opening_pos, ending_pos] : remove_substrings)
		{
			line.erase(line.begin() + opening_pos - offset, line.begin() + ending_pos - offset);
			offset += ending_pos - opening_pos;
		}
	}

	static void trim_whitespace(std::string& line)
	{
		auto start = std::find_if(line.begin(), line.end(), [](unsigned char c) { return !std::isspace(c); });
		if (start == line.end())
		{
			line.clear();
			return;
		}
		auto end = std::find_if(line.rbegin(), line.rend(), [](unsigned char c) { return !std::isspace(c); }).base();
		line = std::string(start, end);
	}

	static void simplify(std::string& line)
	{
		remove_comments(line);
		trim_whitespace(line);
	}

	OlyScriptReadResult read_asset(const char* filepath)
	{
		OlyScriptReadResult result;

		std::vector<std::string> lines = io::read_file_lines(filepath);
		Asset asset;
		
		bool within_asset_segment = false;
		size_t line_number = 0;

		for (auto it = lines.begin(); it != lines.end(); ++it)
		{
			simplify(*it);

			while (it->ends_with('\\'))
			{
				it->pop_back();
				auto next = std::next(it);
				if (next != lines.end())
				{
					simplify(*next);
					it->append(next->begin(), next->end());
					lines.erase(next);
				}
				else
					break;
			}

			if (within_asset_segment)
			{
				if (it->starts_with('$'))
				{
					if (std::string_view(it->begin() + 1, it->end()) == asset.header)
					{
						result.assets.push_back(std::move(asset));
						within_asset_segment = false;
						break;
					}
					else
						LOG << LOG.begin_temp(LOG.warning) << "[L." << line_number << "]" << "Leading $ footer with code " << std::string_view(it->begin() + 1, it->end())
							<< " does not match header code " << asset.header << LOG.end_temp << LOG.nl;
				}
				asset.content.emplace_back(std::move(*it), line_number);
			}
			else
			{
				if (it->starts_with('^'))
				{
					std::vector<std::string_view> tokens = split(*it, ':');
					asset.header = tokens[0].substr(1);
					if (tokens.size() >= 2)
						asset.local_identifier = tokens[1];
					if (tokens.size() >= 3)
						asset.temporary = tokens[2][0] == 'T';

					asset.filepath = filepath;
					within_asset_segment = true;
				}
			}

			++line_number;
		}
		return result;
	}
}
