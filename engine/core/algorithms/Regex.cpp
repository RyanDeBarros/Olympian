#include "Regex.h"

#include "core/algorithms/STLUtils.h"

namespace oly::algo::re
{
	bool first_match(const std::string& input, const std::regex& pattern, std::smatch& match)
	{
		return std::regex_search(input, match, pattern);
	}

	bool first_match(const std::string& input, const char* pattern, std::smatch& match)
	{
		return first_match(input, std::regex(pattern), match);
	}

	std::vector<std::smatch> all_matches(const std::string& input, const char* pattern)
	{
		return all_matches(input, std::regex(pattern));
	}

	std::vector<std::smatch> all_matches(const std::string& input, const std::regex& pattern)
	{
		std::vector<std::smatch> matches;
		auto begin = std::sregex_iterator(input.begin(), input.end(), pattern);
		auto end = std::sregex_iterator();
		for (std::sregex_iterator it = begin; it != end; ++it)
			matches.push_back(*it);
		return matches;
	}

	static const char* vec_prefix = R"(^\s*\(?)";
	static const char* vec_postfix = R"(\)?\s*$)";
	static const char* number = R"((\s*-?\s*\d+\.?\d*\s*))";

	static std::regex vec_pattern(size_t n)
	{
		std::string p;
		p.append(vec_prefix);
		for (size_t i = 0; i < n; ++i)
		{
			if (i > 0)
				p.append(",");
			p.append(number);
		}
		p.append(",?");
		p.append(vec_postfix);
		return std::regex(p.c_str());
	}

	template<glm::length_t N, typename T>
	bool parse_vec(const std::string& input, glm::vec<N, T>& v)
	{
		static std::regex pattern = vec_pattern(N);

		std::smatch match;
		if (!first_match(input, pattern, match))
			return false;

		if (match.size() != N + 1)
			return false;

		glm::vec<N, T> u = v;
		for (size_t i = 0; i < N; ++i)
		{
			std::string num = trim(match[i + 1].str());

			bool negative = false;
			if (num.starts_with('-'))
			{
				negative = true;
				num.erase(num.begin(), num.begin() + 1);
				ltrim(num);
			}

			try
			{
				u[i] = std::stof(num);
			}
			catch (...)
			{
				return false;
			}

			if (negative)
				u[i] = -u[i];
		}
		v = u;

		return true;
	}

	bool parse_float(const std::string& input, float& v)
	{
		glm::vec1 u;
		if (parse_vec(input, u))
		{
			v = u[0];
			return true;
		}
		else
			return false;
	}

	bool parse_vec2(const std::string& input, glm::vec2& v)
	{
		return parse_vec(input, v);
	}

	bool parse_vec3(const std::string& input, glm::vec3& v)
	{
		return parse_vec(input, v);
	}

	bool parse_vec4(const std::string& input, glm::vec4& v)
	{
		return parse_vec(input, v);
	}
}
