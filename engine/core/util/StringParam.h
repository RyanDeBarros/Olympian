#pragma once

#include <string>
#include <string_view>
#include <span>
#include <regex>

#include "core/types/Variant.h"

namespace oly
{
	struct StringParam
	{
	private:
		using VariantType = Variant<std::span<char>, std::span<const char>, std::string>;
		mutable VariantType storage;

	public:
		StringParam() : storage(std::span<char>()) {}
		StringParam(char* string) : storage(std::span<char>(string, strlen(string))) {}
		StringParam(const char* string) : storage(std::span<const char>(string, strlen(string))) {}
		StringParam(const std::span<char> string) : storage(string) {}
		StringParam(const std::span<const char> string) : storage(string) {}
		StringParam(const std::string_view string) : storage(std::span<const char>(string)) {}
		StringParam(const std::string& string) : storage(std::span<const char>(string)) {}
		StringParam(std::string& string) : storage(std::span<char>(string)) {}
		StringParam(std::string&& string) : storage(std::move(string)) {}
		
		StringParam(const StringParam& other) : storage(copy_from(other)) {}
		StringParam(StringParam&& other) noexcept : storage(move_from(std::move(other))) {}

		StringParam& operator=(const StringParam& other)
		{
			if (this != &other)
				storage = copy_from(other);
			return *this;
		}

		StringParam& operator=(StringParam&& other) noexcept
		{
			if (this != &other)
				storage = move_from(std::move(other));
			return *this;
		}

		bool is_mutable() const
		{
			return !storage.holds<std::span<const char>>();
		}

	private:
		static VariantType copy_from(const StringParam& other);
		static VariantType move_from(StringParam&& other) noexcept;

	public:
		std::span<const char> view() const;
		std::span<char> mutview() const;
		std::string copy() const;
		std::string transfer() const;

		StringParam immutable() const;

		size_t size() const noexcept { return view().size(); }
		bool empty() const noexcept { return view().empty(); }

		size_t hash() const;
		bool operator==(const StringParam& other) const;
		friend std::ostream& operator<<(std::ostream& os, const StringParam& string);

		using Iterator = std::span<char>::iterator;
		using ConstIterator = std::span<const char>::iterator;

		Iterator begin() noexcept { return mutview().begin(); }
		Iterator end() noexcept { return mutview().end(); }
		ConstIterator begin() const noexcept { return view().begin(); }
		ConstIterator end() const noexcept { return view().end(); }
		ConstIterator cbegin() const noexcept { return view().begin(); }
		ConstIterator cend() const noexcept { return view().end(); }

		using Match = std::match_results<Iterator>;
		using ConstMatch = std::match_results<ConstIterator>;

		static const size_t NPOS = size_t(-1);

		size_t find(char c, size_t pos = 0, size_t count = NPOS) const;
		size_t rfind(char c, size_t pos = 0, size_t count = NPOS) const;

		void clip(size_t from, size_t count = NPOS) const;
		StringParam substr(size_t from, size_t count = NPOS) const
		{
			StringParam s = *this;
			s.clip(from, count);
			return s;
		}

		void clear() const;
		void erase_before(size_t first) const;
		void erase_after(size_t last) const;

		void pop_front() const
		{
			if (empty())
				throw Error(ErrorCode::InvalidSize);

			erase_before(1);
		}
		
		void pop_back() const
		{
			if (empty())
				throw Error(ErrorCode::InvalidSize);

			erase_after(size() - 1);
		}

		char front() const { return view().front(); }
		char back() const { return view().back(); }

		bool starts_with(char c) const { return front() == c; }
		bool ends_with(char c) const { return back() == c; }

		std::vector<StringParam> split(char delimiter) const;

		static void ltrim(std::string& string);
		StringParam ltrim() const;
		static void rtrim(std::string& string);
		StringParam rtrim() const;
		static void trim(std::string& string);
		StringParam trim() const;

		StringParam to_lower() const;
		StringParam to_upper() const;

		int to_int(const int base = 10) const;
		unsigned int to_uint(const int base = 10) const;
		float to_float() const;
	};

	struct StringParamHeteroHash
	{
		using is_transparent = void;
		size_t operator()(const StringParam& s) const { return s.hash(); }
	};

	struct StringParamHeteroEqual
	{
		using is_transparent = void;
		bool operator()(const StringParam& s1, const StringParam& s2) const { return s1 == s2; }
	};

	inline const StringParam a = StringParam("a");
}

template<>
struct std::hash<oly::StringParam>
{
	size_t operator()(const oly::StringParam& param) const
	{
		return param.hash();
	}
};
