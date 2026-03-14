#pragma once

#include "external/GLM.h"
#include "core/util/StringParam.h"

#include <regex>

namespace oly::algo::re
{
	bool first_match(const StringParam& input, const std::regex& pattern, StringParam::ConstMatch& match);
	bool first_match(StringParam& input, const std::regex& pattern, StringParam::Match& match);
	bool first_match(const StringParam& input, const std::string_view pattern, StringParam::ConstMatch& match);
	bool first_match(StringParam& input, const std::string_view pattern, StringParam::Match& match);
	std::vector<StringParam::ConstMatch> all_matches(const StringParam& input, const std::string_view pattern);
	std::vector<StringParam::Match> all_matches(StringParam& input, const std::string_view pattern);
	std::vector<StringParam::ConstMatch> all_matches(const StringParam& input, const std::regex& pattern);
	std::vector<StringParam::Match> all_matches(StringParam& input, const std::regex& pattern);

	bool parse_float(const StringParam& input, float& v);
	bool parse_vec2(const StringParam& input, glm::vec2& v);
	bool parse_vec3(const StringParam& input, glm::vec3& v);
	bool parse_vec4(const StringParam& input, glm::vec4& v);
}
