#include "StringParam.h"

#include <charconv>
#include <ostream>

namespace oly
{
	StringParam::VariantType StringParam::copy_from(const StringParam& other)
	{
		return other.storage.visit(
			[](std::span<char> string) -> VariantType { return string; },
			[](std::span<const char> string) -> VariantType { return string; },
			[](std::string& string) -> VariantType { return std::span<char>(string); }
		);
	}

	StringParam::VariantType StringParam::move_from(StringParam&& other) noexcept
	{
		return other.storage.visit(
			[](std::span<char> string) -> VariantType { return string; },
			[](std::span<const char> string) -> VariantType { return string; },
			[](std::string& string) -> VariantType { return std::move(string); }
		);
	}

	std::span<const char> StringParam::view() const
	{
		return storage.visit(
			[](const std::span<char> string) -> std::span<const char> { return string; },
			[](const std::span<const char> string) -> std::span<const char> { return string; },
			[](const std::string& string) -> std::span<const char> { return std::span<const char>(string); }
		);
	}

	std::span<char> StringParam::mutview() const
	{
		return storage.visit(
			[](const std::span<char> string) -> std::span<char> { return string; },
			[](const std::span<const char> string) -> std::span<char> { throw Error(ErrorCode::BAD_MUTABILITY); },
			[](std::string& string) -> std::span<char> { return std::span<char>(string); }
		);
	}

	std::string StringParam::copy() const
	{
		return storage.visit(
			[](const std::span<char> string) { return std::string(string.data(), string.size()); },
			[](const std::span<const char> string) { return std::string(string.data(), string.size()); },
			[](const std::string& string) { return string; }
		);
	}

	std::string StringParam::move() const
	{
		return storage.visit(
			[](const std::span<char> string) { return std::string(string.data(), string.size()); },
			[](const std::span<const char> string) { return std::string(string.data(), string.size()); },
			[](std::string& string) { return std::move(string); }
		);
	}

	size_t StringParam::hash() const
	{
		return storage.visit(
			[](const std::span<char> string) {
				size_t h = 0;
				for (char c : string) {
					h = h * 31 + std::hash<char>{}(c);
				}
				return h;
			},
			[](const std::span<const char> string) {
				size_t h = 0;
				for (char c : string) {
					h = h * 31 + std::hash<char>{}(c);
				}
				return h;
			},
			[](const std::string& string) { return std::hash<std::string>{}(string); }
		);
	}

	bool StringParam::operator==(const StringParam& other) const
	{
		std::span<const char> my_span = view();
		std::span<const char> their_span = other.view();
		if (my_span.size() != their_span.size())
			return false;

		for (size_t i = 0; i < my_span.size(); ++i)
			if (my_span[i] != their_span[i])
				return false;
		return true;
	}

	std::ostream& operator<<(std::ostream& os, const StringParam& string)
	{
		if (std::string* str = string.storage.safe_get<std::string>())
			os << *str;
		else
		{
			std::span<const char> span = string.view();
			for (char c : span)
				os << c;
		}
		return os;
	}

	size_t StringParam::find(char c, size_t pos, size_t count) const
	{
		std::span<const char> span = view();
		if (pos >= span.size() || count == 0)
			return NPOS;

		auto begin = span.begin() + pos;
		auto end = span.begin() + std::min(span.size(), pos + count);

		auto it = std::find(begin, end, c);
		if (it == end)
			return NPOS;

		return std::distance(span.begin(), it);
	}

	size_t StringParam::rfind(char c, size_t pos, size_t count) const
	{
		std::span<const char> span = view();
		if (pos >= span.size() || count == 0)
			return NPOS;

		auto rbegin = std::make_reverse_iterator(span.begin() + std::min(span.size(), pos + count));
		auto rend = std::make_reverse_iterator(span.begin() + pos);

		auto it = std::find(rbegin, rend, c);
		if (it == rend)
			return NPOS;

		return std::distance(span.begin(), it.base()) - 1;
	}

	void StringParam::clip(size_t from, size_t count) const
	{
		size_t sz = size();
		if (from < sz)
		{
			count = std::min(count, sz - from);
			storage.visit(
				[from, count](std::span<char>& span) { span = span.subspan(from, count); },
				[from, count](std::span<const char>& span) { span = span.subspan(from, count); },
				[from, count](std::string& string) { string.erase(string.begin() + from + count, string.end()); string.erase(string.begin(), string.begin() + from); }
			);
		}
		else
			clear();
	}

	void StringParam::clear() const
	{
		storage.visit(
			[](std::span<char>& span) { span = {}; },
			[](std::span<const char>& span) { span = {}; },
			[](std::string& string) { string = {}; }
		);
	}

	void StringParam::erase_before(size_t first) const
	{
		if (first < size())
		{
			storage.visit(
				[first](std::span<char>& span) { span = span.subspan(first); },
				[first](std::span<const char>& span) { span = span.subspan(first); },
				[first](std::string& string) { string.erase(string.begin(), string.begin() + first); }
			);
		}
		else
			clear();
	}

	void StringParam::erase_after(size_t last) const
	{
		if (last < size())
		{
			storage.visit(
				[last](std::span<char>& span) { span = span.subspan(0, last); },
				[last](std::span<const char>& span) { span = span.subspan(0, last); },
				[last](std::string& string) { string.erase(string.begin() + last, string.end()); }
			);
		}
	}

	std::vector<StringParam> StringParam::split(char delimiter) const
	{
		std::span<const char> span = view();
		std::vector<StringParam> result;
		size_t start = 0;
		while (start < span.size())
		{
			size_t end = find(delimiter, start);
			if (end == StringParam::NPOS)
			{
				result.emplace_back(span.subspan(start));
				break;
			}
			result.emplace_back(span.subspan(start, end - start));
			start = end + 1;
		}
		return result;
	}

	void StringParam::ltrim(std::string& string)
	{
		size_t start = 0;
		while (start < string.size() && std::isspace(string[start]))
			++start;

		string.erase(0, start);
	}

	StringParam StringParam::ltrim() const
	{
		std::span<const char> span = view();
		if (span.empty())
			return *this;

		size_t start = 0;
		while (start < span.size() && std::isspace(span[start]))
			++start;

		clip(start);
		return *this;
	}

	void StringParam::rtrim(std::string& string)
	{
		size_t end = string.size();
		while (end > 0 && std::isspace(string[end - 1]))
			--end;

		string.erase(end);
	}

	StringParam StringParam::rtrim() const
	{
		std::span<const char> span = view();
		if (span.empty())
			return *this;

		size_t end = span.size();
		while (end > 0 && std::isspace(span[end - 1]))
			--end;

		clip(0, end);
		return *this;
	}

	void StringParam::trim(std::string& string)
	{
		size_t start = 0;
		while (start < string.size() && std::isspace(string[start]))
			++start;

		size_t end = string.size();
		while (end > start && std::isspace(string[end - 1]))
			--end;

		string.erase(end);
		string.erase(0, start);
	}

	StringParam StringParam::trim() const
	{
		std::span<const char> span = view();
		if (span.empty())
			return *this;

		size_t start = 0;
		while (start < span.size() && std::isspace(span[start]))
			++start;

		size_t end = span.size();
		while (end > start && std::isspace(span[end - 1]))
			--end;

		clip(start, end - start);
		return *this;
	}

	StringParam StringParam::to_lower() const
	{
		std::span<char> span = mutview();
		if (span.empty())
			return *this;

		for (char& c : span)
			c = (char)(std::tolower(c));

		return *this;
	}

	StringParam StringParam::to_upper() const
	{
		std::span<char> span = mutview();
		if (span.empty())
			return *this;

		for (char& c : span)
			c = (char)(std::toupper(c));

		return *this;
	}

	int StringParam::to_int() const
	{
		std::span<const char> span = view();
		int v;
		auto [_, ec] = std::from_chars(span.data(), span.data() + span.size(), v);
		if (ec == std::errc())
			return v;
		else
			throw Error(ErrorCode::CANNOT_PARSE, std::to_string((int)ec));
	}

	unsigned int StringParam::to_uint() const
	{
		std::span<const char> span = view();
		unsigned int v;
		auto [_, ec] = std::from_chars(span.data(), span.data() + span.size(), v);
		if (ec == std::errc())
			return v;
		else
			throw Error(ErrorCode::CANNOT_PARSE, std::to_string((int)ec));
	}

	float StringParam::to_float() const
	{
		std::span<const char> span = view();
		float v;
		auto [_, ec] = std::from_chars(span.data(), span.data() + span.size(), v);
		if (ec == std::errc())
			return v;
		else
			throw Error(ErrorCode::CANNOT_PARSE, std::to_string((int)ec));
	}
}
