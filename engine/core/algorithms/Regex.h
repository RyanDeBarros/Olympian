#pragma once

#include "external/GLM.h"

#include <regex>

namespace oly::algo::re
{
	bool first_match(const std::string& input, const std::regex& pattern, std::smatch& match);
	bool first_match(const std::string& input, const char* pattern, std::smatch& match);
	std::vector<std::smatch> all_matches(const std::string& input, const char* pattern);
	std::vector<std::smatch> all_matches(const std::string& input, const std::regex& pattern);

	bool parse_float(const std::string& input, float& v);
	bool parse_vec2(const std::string& input, glm::vec2& v);
	bool parse_vec3(const std::string& input, glm::vec3& v);
	bool parse_vec4(const std::string& input, glm::vec4& v);
}
