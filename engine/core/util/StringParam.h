#pragma once

#include "core/base/Errors.h"

#include <imp/traits.hpp>
#include <imp/variant.hpp>

#include <regex>
#include <span>
#include <string>
#include <string_view>

namespace oly
{
    struct StringParam
    {
        friend std::ostream& operator<<(std::ostream& os, const StringParam& string);

    private:
        using VariantType = imp::variant<std::span<char>, std::span<const char>, std::string>;

    public:
        using Iterator = std::span<char>::iterator;
        using ConstIterator = std::span<const char>::iterator;

        using Match = std::match_results<Iterator>;
        using ConstMatch = std::match_results<ConstIterator>;

    private:
        mutable VariantType storage_;

    public:
        static const size_t kNPos = size_t(-1);

        StringParam();
        StringParam(char* string);
        StringParam(const char* string);
        StringParam(const std::span<char> string);
        StringParam(const std::span<const char> string);
        StringParam(const std::string_view string);
        StringParam(const std::string& string);
        StringParam(std::string& string);
        StringParam(std::string&& string);

        StringParam(const StringParam& o);
        StringParam(StringParam&& o) noexcept;

        template<imp::numeric T>
        StringParam(T n)
            : storage_(std::to_string(n))
        {
        }

        StringParam& operator=(const StringParam& o);
        StringParam& operator=(StringParam&& o) noexcept;

    public:
        bool is_mutable() const;

        std::span<const char> view() const;
        std::span<char> mutview() const;
        std::string copy() const;
        std::string transfer() const;

        StringParam immutable() const;

        size_t size() const noexcept;
        bool empty() const noexcept;

        size_t hash() const;
        bool operator==(const StringParam& o) const;

        Iterator begin() noexcept;
        Iterator end() noexcept;
        ConstIterator begin() const noexcept;
        ConstIterator end() const noexcept;
        ConstIterator cbegin() const noexcept;
        ConstIterator cend() const noexcept;

        size_t find(char c, size_t pos = 0, size_t count = kNPos) const;
        size_t rfind(char c, size_t pos = 0, size_t count = kNPos) const;

        void clip(size_t from, size_t count = kNPos) const;
        StringParam substr(size_t from, size_t count = kNPos) const;

        void clear() const;
        void erase_before(size_t first) const;
        void erase_after(size_t last) const;

        void pop_front() const;
        void pop_back() const;

        char front() const;
        char back() const;

        bool starts_with(char c) const;
        bool ends_with(char c) const;

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

    private:
        static VariantType copy_from(const StringParam& o);
        static VariantType move_from(StringParam&& o) noexcept;
    };
}

template<>
struct std::hash<oly::StringParam>
{
    size_t operator()(const oly::StringParam& param) const;
};
