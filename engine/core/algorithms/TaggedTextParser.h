#pragma once

#include "core/util/UTF.h"

#include <vector>
#include <stack>

namespace oly::algo
{
	struct UTFTaggedTextParser
	{
		struct Group
		{
			utf::String str;
			std::stack<utf::String> tags;
		};

		std::vector<Group> groups;

		UTFTaggedTextParser(const utf::String& input);
	};
}
