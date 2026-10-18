#include "DeferredStringParam.h"

namespace oly
{
    DeferredStringList::DeferredStringList(std::initializer_list<std::string> list)
        : list(std::move(list))
    {
    }

    size_t DeferredStringList::size() const
    {
        return list.size();
    }

    bool DeferredStringList::empty() const
    {
        return list.empty();
    }

    const std::string& DeferredStringList::operator[](size_t i) const
    {
        return list[i];
    }

    void DeferredStringList::push_back(std::string string)
    {
        list.push_back(std::move(string));
    }

    auto DeferredStringList::begin()
    {
        return list.begin();
    }

    auto DeferredStringList::begin() const
    {
        return list.begin();
    }

    auto DeferredStringList::cbegin() const
    {
        return list.cbegin();
    }

    auto DeferredStringList::end()
    {
        return list.end();
    }

    auto DeferredStringList::end() const
    {
        return list.end();
    }

    auto DeferredStringList::cend() const
    {
        return list.cend();
    }

    DeferredStringParam::DeferredStringParam()
        : data_(std::string())
    {
    }

    DeferredStringParam::DeferredStringParam(const StringParam& string)
        : data_(string)
    {
    }

    DeferredStringParam::DeferredStringParam(StringParam&& string)
        : data_(std::move(string))
    {
    }

    DeferredStringParam::DeferredStringParam(const std::vector<StringParam>& strings)
        : data_(strings)
    {
    }

    DeferredStringParam::DeferredStringParam(std::vector<StringParam>&& strings)
        : data_(std::move(strings))
    {
    }

    DeferredStringParam::DeferredStringParam(std::initializer_list<StringParam> list)
        : data_(std::vector<StringParam>(list))
    {
    }


    DeferredStringParam::DeferredStringParam(const DeferredStringList& strings)
    {
        if (strings.size() == 1)
            data_ = strings[0];
        else if (strings.size() > 1)
        {
            data_ = std::vector<StringParam>();
            for (const std::string& string : strings)
                get_list().push_back(string);
        }
        else
            data_ = std::string();
    }

    DeferredStringParam::DeferredStringParam(DeferredStringList&& strings)
    {
        if (strings.size() == 1)
            data_ = std::move(strings[0]);
        else if (strings.size() > 1)
        {
            data_ = std::vector<StringParam>();
            for (std::string& string : strings)
                get_list().push_back(std::move(string));
        }
        else
            data_ = std::string();
    }

    StringParam DeferredStringParam::str() const
    {
        convert_to_single();
        return get_single();
    }

    bool DeferredStringParam::empty() const
    {
        return data_.visit(
            [](const StringParam& s) { return s.empty(); },
            [](const std::vector<StringParam>& v) { return v.empty(); }
        );
    }

    DeferredStringParam& DeferredStringParam::operator<<(const StringParam& string)
    {
        if (string.empty())
            return *this;

        convert_to_list();
        get_list().push_back(string);
        return *this;
    }

    DeferredStringParam& DeferredStringParam::operator<<(StringParam&& string)
    {
        if (string.empty())
            return *this;

        convert_to_list();
        get_list().push_back(std::move(string));
        return *this;
    }

    DeferredStringParam& DeferredStringParam::operator<<(const DeferredStringParam& param)
    {
        if (param.empty())
            return *this;

        convert_to_list();
        param.data_.visit(
            [this](const StringParam& s) { get_list().push_back(s); },
            [this](const std::vector<StringParam>& v) { get_list().insert(get_list().end(), v.begin(), v.end()); }
        );
        return *this;
    }

    DeferredStringParam& DeferredStringParam::operator<<(DeferredStringParam&& param)
    {
        if (param.empty())
            return *this;

        convert_to_list();
        param.data_.visit(
            [this](StringParam& s) { get_list().push_back(std::move(s)); },
            [this](std::vector<StringParam>& v) { get_list().insert(get_list().end(), std::make_move_iterator(v.begin()), std::make_move_iterator(v.end())); }
        );
        return *this;
    }

    void DeferredStringParam::convert_to_single() const
    {
        if (auto v = data_.safe_get<std::vector<StringParam>>())
        {
            std::stringstream ss;
            for (StringParam& s : *v)
                ss << s.transfer();
            data_ = StringParam(ss.str());
        }
    }

    void DeferredStringParam::convert_to_list() const
    {
        if (auto v = data_.safe_get<StringParam>())
        {
            std::vector<StringParam> vec;
            vec.push_back(std::move(*v));
            data_ = std::move(vec);
        }
    }

    StringParam& DeferredStringParam::get_single() const
    {
        return data_.get<StringParam>();
    }

    std::vector<StringParam>& DeferredStringParam::get_list() const
    {
        return data_.get<std::vector<StringParam>>();
    }

    DeferredStringList& operator<<(DeferredStringList& list, const DeferredStringParam& param)
    {
        if (const StringParam* string = param.data_.safe_get<StringParam>())
            list.push_back(string->copy());
        else
        {
            for (const StringParam& string : param.get_list())
                list.push_back(string.copy());
        }
        return list;
    }

    DeferredStringList& operator<<(DeferredStringList& list, DeferredStringParam&& param)
    {
        if (StringParam* string = param.data_.safe_get<StringParam>())
            list.push_back(string->transfer());
        else
        {
            for (StringParam& string : param.get_list())
                list.push_back(string.transfer());
        }
        return list;
    }

    DeferredStringList& operator<<(DeferredStringList& list, const std::string& str)
    {
        list.push_back(str);
        return list;
    }

    DeferredStringList& operator<<(DeferredStringList& list, std::string&& str)
    {
        list.push_back(std::move(str));
        return list;
    }

    DeferredStringList operator<<(DeferredStringList&& list, const DeferredStringParam& param)
    {
        if (const StringParam* string = param.data_.safe_get<StringParam>())
            list.push_back(string->copy());
        else
        {
            for (const StringParam& string : param.get_list())
                list.push_back(string.copy());
        }
        return std::move(list);
    }

    DeferredStringList operator<<(DeferredStringList&& list, DeferredStringParam&& param)
    {
        if (StringParam* string = param.data_.safe_get<StringParam>())
            list.push_back(string->transfer());
        else
        {
            for (StringParam& string : param.get_list())
                list.push_back(string.transfer());
        }
        return std::move(list);
    }

    DeferredStringList operator<<(DeferredStringList&& list, const std::string& str)
    {
        list.push_back(str);
        return std::move(list);
    }

    DeferredStringList operator<<(DeferredStringList&& list, std::string&& str)
    {
        list.push_back(std::move(str));
        return std::move(list);
    }
}
