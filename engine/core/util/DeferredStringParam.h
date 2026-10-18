#pragma once

#include "core/util/StringParam.h"

#include <sstream>

namespace oly
{
    struct DeferredStringList
    {
        std::vector<std::string> list;

        DeferredStringList(std::initializer_list<std::string> list);

        size_t size() const;
        bool empty() const;
        const std::string& operator[](size_t i) const;
        void push_back(std::string string);

        auto begin();
        auto begin() const;
        auto cbegin() const;
        auto end();
        auto end() const;
        auto cend() const;
    };

    class DeferredStringParam
    {
        friend DeferredStringList& operator<<(DeferredStringList&, const DeferredStringParam&);
        friend DeferredStringList& operator<<(DeferredStringList&, DeferredStringParam&&);
        friend DeferredStringList operator<<(DeferredStringList&&, const DeferredStringParam&);
        friend DeferredStringList operator<<(DeferredStringList&&, DeferredStringParam&&);

        using V = imp::variant<StringParam, std::vector<StringParam>>;

    private:
        mutable V data_;

    public:
        DeferredStringParam();
        DeferredStringParam(const StringParam& string);
        DeferredStringParam(StringParam&& string);
        DeferredStringParam(const std::vector<StringParam>& strings);
        DeferredStringParam(std::vector<StringParam>&& strings);
        DeferredStringParam(std::initializer_list<StringParam> list);

        template<typename... Args, typename = std::enable_if_t<(sizeof...(Args) > 1)>>
            DeferredStringParam(Args&&... args)
            : data_(std::vector<StringParam>{ StringParam(std::forward<Args>(args))... })
        {
        }

        DeferredStringParam(const DeferredStringList& strings);
        DeferredStringParam(DeferredStringList&& strings);

    public:
        StringParam str() const;
        bool empty() const;

        DeferredStringParam& operator<<(const StringParam& string);
        DeferredStringParam& operator<<(StringParam&& string);
        DeferredStringParam& operator<<(const DeferredStringParam& param);
        DeferredStringParam& operator<<(DeferredStringParam&& param);

    private:
        void convert_to_single() const;
        void convert_to_list() const;
        StringParam& get_single() const;
        std::vector<StringParam>& get_list() const;
    };

    extern DeferredStringList& operator<<(DeferredStringList& list, const DeferredStringParam& param);
    extern DeferredStringList& operator<<(DeferredStringList& list, DeferredStringParam&& param);
    extern DeferredStringList& operator<<(DeferredStringList& list, const std::string& str);
    extern DeferredStringList& operator<<(DeferredStringList& list, std::string&& str);
    extern DeferredStringList operator<<(DeferredStringList&& list, const DeferredStringParam& param);
    extern DeferredStringList operator<<(DeferredStringList&& list, DeferredStringParam&& param);
    extern DeferredStringList operator<<(DeferredStringList&& list, const std::string& str);
    extern DeferredStringList operator<<(DeferredStringList&& list, std::string&& str);
}
