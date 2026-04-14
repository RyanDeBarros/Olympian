#pragma once

#include "core/util/StringParam.h"

#include <sstream>

namespace oly
{
	using DeferredStringList = std::vector<std::string>;

	class DeferredStringParam
	{
		using V = Variant<StringParam, std::vector<StringParam>>;
		mutable V data;

	public:
		DeferredStringParam() : data(std::string()) {}
		DeferredStringParam(const StringParam& string) : data(string) {}
		DeferredStringParam(StringParam&& string) : data(std::move(string)) {}
		DeferredStringParam(const std::vector<StringParam>& strings) : data(strings) {}
		DeferredStringParam(std::vector<StringParam>&& strings) : data(std::move(strings)) {}
		DeferredStringParam(std::initializer_list<StringParam> list) : data(std::vector<StringParam>(list)) {}

		template<typename... Args, typename = std::enable_if_t<(sizeof...(Args) > 1)>>
		DeferredStringParam(Args&&... args) : data(std::vector<StringParam>{ StringParam(std::forward<Args>(args))... }) {}

		DeferredStringParam(const DeferredStringList& strings)
		{
			if (strings.size() == 1)
				data = strings[0];
			else if (strings.size() > 1)
			{
				data = std::vector<StringParam>();
				for (const std::string& string : strings)
					get_list().push_back(string);
			}
			else
				data = std::string();
		}

		DeferredStringParam(DeferredStringList&& strings)
		{
			if (strings.size() == 1)
				data = std::move(strings[0]);
			else if (strings.size() > 1)
			{
				data = std::vector<StringParam>();
				for (std::string& string : strings)
					get_list().push_back(std::move(string));
			}
			else
				data = std::string();
		}

		StringParam str() const
		{
			convert_to_single();
			return get_single();
		}

		bool empty() const
		{
			return data.visit(
				[](const StringParam& s) { return s.empty(); },
				[](const std::vector<StringParam>& v) { return v.empty(); }
			);
		}

		DeferredStringParam& operator<<(const StringParam& string)
		{
			if (string.empty())
				return *this;

			convert_to_list();
			get_list().push_back(string);
			return *this;
		}

		DeferredStringParam& operator<<(StringParam&& string)
		{
			if (string.empty())
				return *this;

			convert_to_list();
			get_list().push_back(std::move(string));
			return *this;
		}

		friend DeferredStringList& operator<<(DeferredStringList&, const DeferredStringParam&);
		friend DeferredStringList& operator<<(DeferredStringList&, DeferredStringParam&&);
		friend DeferredStringList operator<<(DeferredStringList&&, const DeferredStringParam&);
		friend DeferredStringList operator<<(DeferredStringList&&, DeferredStringParam&&);

		DeferredStringParam& operator<<(const DeferredStringParam& param)
		{
			if (param.empty())
				return *this;

			convert_to_list();
			param.data.visit(
				[this](const StringParam& s) { get_list().push_back(s); },
				[this](const std::vector<StringParam>& v) { get_list().insert(get_list().end(), v.begin(), v.end()); }
			);
			return *this;
		}

		DeferredStringParam& operator<<(DeferredStringParam&& param)
		{
			if (param.empty())
				return *this;

			convert_to_list();
			param.data.visit(
				[this](StringParam& s) { get_list().push_back(std::move(s)); },
				[this](std::vector<StringParam>& v) { get_list().insert(get_list().end(), std::make_move_iterator(v.begin()), std::make_move_iterator(v.end())); }
			);
			return *this;
		}

	private:
		void convert_to_single() const
		{
			if (auto v = data.safe_get<std::vector<StringParam>>())
			{
				std::stringstream ss;
				for (StringParam& s : *v)
					ss << s.transfer();
				data = StringParam(ss.str());
			}
		}

		void convert_to_list() const
		{
			if (auto v = data.safe_get<StringParam>())
			{
				std::vector<StringParam> vec;
				vec.push_back(std::move(*v));
				data = std::move(vec);
			}
		}

		StringParam& get_single() const
		{
			return data.get<StringParam>();
		}

		std::vector<StringParam>& get_list() const
		{
			return data.get<std::vector<StringParam>>();
		}
	};

	inline DeferredStringList& operator<<(DeferredStringList& list, const DeferredStringParam& param)
	{
		if (const StringParam* string = param.data.safe_get<StringParam>())
			list.push_back(string->copy());
		else
		{
			for (const StringParam& string : param.get_list())
				list.push_back(string.copy());
		}
		return list;
	}

	inline DeferredStringList& operator<<(DeferredStringList& list, DeferredStringParam&& param)
	{
		if (StringParam* string = param.data.safe_get<StringParam>())
			list.push_back(string->transfer());
		else
		{
			for (StringParam& string : param.get_list())
				list.push_back(string.transfer());
		}
		return list;
	}

	inline DeferredStringList operator<<(DeferredStringList&& list, const DeferredStringParam& param)
	{
		if (const StringParam* string = param.data.safe_get<StringParam>())
			list.push_back(string->copy());
		else
		{
			for (const StringParam& string : param.get_list())
				list.push_back(string.copy());
		}
		return std::move(list);
	}

	inline DeferredStringList operator<<(DeferredStringList&& list, DeferredStringParam&& param)
	{
		if (StringParam* string = param.data.safe_get<StringParam>())
			list.push_back(string->transfer());
		else
		{
			for (StringParam& string : param.get_list())
				list.push_back(string.transfer());
		}
		return std::move(list);
	}
}
