#include "LabelRegistry.h"

#include "util/Hash.h"

#include <algorithm>

namespace oly::editor
{
	static LabelRegistry::DataStructure LABEL_REGISTRY;
	static LabelSpanRegistry::DataStructure LABEL_SPAN_REGISTRY;

	LabelRegistry::Handle LabelRegistry::Intern(const std::string_view label)
	{
		return LABEL_REGISTRY.Intern(label);
	}

	const char* LabelRegistry::String(const Handle handle)
	{
		return LABEL_REGISTRY.Get(handle).c_str();
	}

	struct LabelSpanHelper
	{
		size_t operator()(const std::span<std::string_view>& span) const
		{
			Hasher h;
			for (const auto& s : span)
				h.with(s);
			return h;
		}

		bool operator()(const std::vector<std::string>& a, const std::span<std::string_view>& b) const
		{
			return std::ranges::equal(a, b);
		}
	};

	struct LabelSpanConversion
	{
		std::vector<std::string> operator()(const std::span<std::string_view>& span) const
		{
			std::vector<std::string> v;
			v.reserve(span.size());
			for (const auto& s : span)
				v.push_back(std::string(s));
			return v;
		}
	};

	LabelSpanRegistry::Handle LabelSpanRegistry::Intern(const std::span<std::string_view> labels)
	{
		return LABEL_SPAN_REGISTRY.Intern<decltype(labels), LabelSpanHelper, LabelSpanHelper, LabelSpanConversion>(labels);
	}
	
	const char* LabelSpanRegistry::String(const Handle handle, size_t i)
	{
		return LABEL_SPAN_REGISTRY.Get(handle)[i].c_str();
	}
	
	size_t LabelSpanRegistry::Count(const Handle handle)
	{
		return LABEL_SPAN_REGISTRY.Get(handle).size();
	}
}
