#pragma once

#include "core/util/StringParam.h"

#include "external/GLM.h"

#include <regex>

namespace oly::algo::re
{
    extern bool first_match(const StringParam& input, const std::regex& pattern, StringParam::ConstMatch& match);
    extern bool first_match(StringParam& input, const std::regex& pattern, StringParam::Match& match);
    extern bool first_match(const StringParam& input, const std::string_view pattern, StringParam::ConstMatch& match);
    extern bool first_match(StringParam& input, const std::string_view pattern, StringParam::Match& match);

    extern std::vector<StringParam::ConstMatch> all_matches(const StringParam& input, const std::string_view pattern);
    extern std::vector<StringParam::Match> all_matches(StringParam& input, const std::string_view pattern);
    extern std::vector<StringParam::ConstMatch> all_matches(const StringParam& input, const std::regex& pattern);
    extern std::vector<StringParam::Match> all_matches(StringParam& input, const std::regex& pattern);

    template<typename T>
    bool try_parse(const StringParam& input, T& v);
}
