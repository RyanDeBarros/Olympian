#pragma once

#include <string>

namespace oly
{
	namespace utf
	{
		extern std::u8string encode(const std::u16string& utf16, bool ignore_invalid_chars = false);
		extern std::u16string decode_utf16(const std::u8string& utf8);
		extern std::u8string encode(const std::u32string& utf32, bool ignore_invalid_chars = false);
		extern std::u32string decode_utf32(const std::u8string& utf8);
		extern std::u8string convert(const std::string& str);
		extern std::string convert(const std::u8string& utf8);

		class Codepoint
		{
			int c;

		public:
			constexpr Codepoint() : c(0) {}
			constexpr explicit Codepoint(int c) : c(c) {}
			operator int() const { return c; }

			constexpr bool operator==(const Codepoint&) const = default;
			constexpr bool operator==(int x) const { return c == x; }
			constexpr bool operator==(char x) const { return c == x; }
		};

		constexpr bool is_n_or_r(Codepoint codepoint) { return codepoint == '\n' || codepoint == '\r'; }
		constexpr bool is_rn(Codepoint r, Codepoint n) { return r == '\r' && n == '\n'; }

		class String
		{
			friend class Iterator;
			std::u8string str = u8"";

		public:
			String(const std::u8string& str) : str(str) {}
			String(std::u8string&& str) : str(std::move(str)) {}
			String(const std::string& str) : str(convert(str)) {}
			String(const std::u16string& str) : str(encode(str)) {}
			String(const std::u32string& str) : str(encode(str)) {}
			String(const char8_t* str) : str(str) {}
			String(const char* str) : str(convert(str)) {}
			String(const char16_t* str) : str(encode(std::u16string(str))) {}
			String(const char32_t* str) : str(encode(std::u32string(str))) {}
			String() = default;
			String(const String&) = default;
			String(String&&) noexcept = default;
			String& operator=(const String&) = default;
			String& operator=(String&&) noexcept = default;

			struct Iterator
			{
			private:
				friend class String;
				const String& string;
				size_t i;

			public:
				Iterator(const String& string, size_t i) : string(string), i(i) {}
				Iterator(const Iterator&) = default;
				Iterator(Iterator&&) noexcept = default;
				Iterator& operator=(const Iterator&) = default;
				Iterator& operator=(Iterator&&) noexcept = default;

				Codepoint codepoint() const;
				Iterator& operator++();
				Iterator operator++(int);
				Iterator& operator--();
				Iterator operator--(int);
				bool operator==(const Iterator& other) const { return string.str == other.string.str && i == other.i; }
				bool operator!=(const Iterator& other) const { return string.str != other.string.str || i != other.i; }
				char num_bytes() const;
				operator bool() const { return i < string.str.size(); }
				Codepoint advance();
			};

			Iterator begin() const { return Iterator(*this, 0); }
			Iterator end() const { return Iterator(*this, str.size()); }
			size_t size() const { return str.size(); }
			bool empty() const { return str.empty(); }
			std::u8string& encoding() { return str; }
			const std::u8string& encoding() const { return str; }

			void push_back(Codepoint codepoint);

			String operator+(const String& rhs) const;
			String& operator+=(const String& rhs);
			String operator*(size_t n) const;
			String& operator*=(size_t n);
			bool operator==(const String& other) const { return str == other.str; }
			bool operator!=(const String& other) const { return str != other.str; }
			size_t hash() const { return std::hash<std::u8string>{}(str); }

			String(const Iterator& begin_, const Iterator& end_);
			String substr(size_t begin_, size_t end_) const;
		};
	}
}

template<> struct std::hash<oly::utf::String> { size_t operator()(const oly::utf::String& string) const { return string.hash(); } };
template<> struct std::hash<oly::utf::Codepoint> { size_t operator()(const oly::utf::Codepoint& c) const { return std::hash<int>{}(c); } };
