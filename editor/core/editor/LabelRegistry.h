#pragma once

#include "util/Internship.h"

#include <span>
#include <string_view>

namespace oly::editor
{
	struct LabelRegistry
	{
		using DataStructure = Internship<std::string>;
		using Handle = DataStructure::Handle;

		static Handle Intern(const std::string_view label);
		static const char* String(const Handle handle);
	};

	struct LabelSpanRegistry
	{
		using DataStructure = Internship<std::vector<std::string>>;
		using Handle = DataStructure::Handle;

		static Handle Intern(const std::span<std::string_view> labels);
		static const char* String(const Handle handle, size_t i);
		static size_t Count(const Handle handle);
	};
}
