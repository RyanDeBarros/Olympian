#pragma once

#include <vector>
#include <string>

namespace oly::assets
{
	struct Asset
	{
		std::string header;
		std::string filepath;
		std::string local_identifier;
		bool temporary;
		
		struct Line
		{
			std::string line;
			size_t number;
		};

		std::vector<Line> content;
	};

	struct OlyScriptReadResult
	{
		// TODO v3 add versioning system to script
		std::vector<Asset> assets;
	};

	extern OlyScriptReadResult read_asset(const char* filepath);
	inline OlyScriptReadResult read_asset(const std::string& filepath) { return read_asset(filepath.c_str()); }
}
