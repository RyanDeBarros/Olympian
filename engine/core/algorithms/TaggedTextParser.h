#pragma once

#include "core/util/UTF.h"

#include <vector>

namespace oly::algo
{
	struct UTFTaggedTextParser
	{
		struct Group
		{
			utf::String str;
			std::vector<utf::String> tags;
		};

		std::vector<Group> groups;

		UTFTaggedTextParser(const utf::String& input);
	};
}
