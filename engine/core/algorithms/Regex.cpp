#include "Regex.h"

#include "core/base/Color.h"

#include <sstream>

namespace oly::algo::re
{
	bool first_match(const StringParam& input, const std::regex& pattern, StringParam::ConstMatch& match)
	{
		return std::regex_search(input.begin(), input.end(), match, pattern);
	}

	bool first_match(StringParam& input, const std::regex& pattern, StringParam::Match& match)
	{
		return std::regex_search(input.begin(), input.end(), match, pattern);
	}

	bool first_match(const StringParam& input, const std::string_view pattern, StringParam::ConstMatch& match)
	{
		return first_match(input, std::regex(pattern.data()), match);
	}

	bool first_match(StringParam& input, const std::string_view pattern, StringParam::Match& match)
	{
		return first_match(input, std::regex(pattern.data()), match);
	}

	std::vector<StringParam::ConstMatch> all_matches(const StringParam& input, const std::string_view pattern)
	{
		return all_matches(input, std::regex(pattern.data()));
	}

	std::vector<StringParam::Match> all_matches(StringParam& input, const std::string_view pattern)
	{
		return all_matches(input, std::regex(pattern.data()));
	}

	std::vector<StringParam::ConstMatch> all_matches(const StringParam& input, const std::regex& pattern)
	{
		std::vector<StringParam::ConstMatch> matches;
		auto begin = std::regex_iterator(input.begin(), input.end(), pattern);
		auto end = std::regex_iterator<StringParam::ConstIterator>();
		for (std::regex_iterator it = begin; it != end; ++it)
			matches.push_back(*it);
		return matches;
	}

	std::vector<StringParam::Match> all_matches(StringParam& input, const std::regex& pattern)
	{
		std::vector<StringParam::Match> matches;
		auto begin = std::regex_iterator(input.begin(), input.end(), pattern);
		auto end = std::regex_iterator<StringParam::Iterator>();
		for (std::regex_iterator it = begin; it != end; ++it)
			matches.push_back(*it);
		return matches;
	}

	static std::regex vec_pattern(size_t n)
	{
		std::stringstream p;
		p << R"(^\s*\(?)";
		for (size_t i = 0; i < n; ++i)
		{
			if (i > 0)
				p << ",";
			p << R"((\s*-?\s*\d+\.?\d*\s*))";
		}
		p << ",?" << R"(\)?\s*$)";
		return std::regex(p.str());
	}

	template<glm::length_t N, typename T>
	bool parse_vec(const StringParam& input, glm::vec<N, T>& v)
	{
		static std::regex pattern = vec_pattern(N);

		StringParam::ConstMatch match;
		if (!first_match(input, pattern, match))
			return false;

		if (match.size() != N + 1)
			return false;

		glm::vec<N, T> u = v;
		for (size_t i = 0; i < N; ++i)
		{
			StringParam num = match[i + 1].str();
			num.trim();

			bool negative = false;
			if (num.starts_with('-'))
			{
				negative = true;
				num.clip(1);
				num.ltrim();
			}

			try
			{
				u[i] = StringParam(num).to_float();
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

	template<>
	bool try_parse<float>(const StringParam& input, float& v)
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

	template<>
	bool try_parse<glm::vec2>(const StringParam& input, glm::vec2& v)
	{
		return parse_vec(input, v);
	}

	template<>
	bool try_parse<glm::vec3>(const StringParam& input, glm::vec3& v)
	{
		return parse_vec(input, v);
	}

	template<>
	bool try_parse<glm::vec4>(const StringParam& input, glm::vec4& v)
	{
		return parse_vec(input, v);
	}

	template<>
	bool try_parse<Color>(const StringParam& text, Color& color)
	{
		text.to_lower();
		if (text == "white")
		{
			color = colors::WHITE;
			return true;
		}
		else if (text == "black")
		{
			color = colors::BLACK;
			return true;
		}
		else if (text == "red")
		{
			color = colors::RED;
			return true;
		}
		else if (text == "green")
		{
			color = colors::GREEN;
			return true;
		}
		else if (text == "blue")
		{
			color = colors::BLUE;
			return true;
		}
		else if (text == "cyan")
		{
			color = colors::CYAN;
			return true;
		}
		else if (text == "magenta")
		{
			color = colors::MAGENTA;
			return true;
		}
		else if (text == "yellow")
		{
			color = colors::YELLOW;
			return true;
		}
		else if (text == "orange")
		{
			color = colors::ORANGE;
			return true;
		}
		else
		{
			glm::vec4 c;
			if (parse_vec(text, c))
			{
				color = c;
				return true;
			}
			else
				return false;
		}
	}
}
