#pragma once

#include <imp/utf.hpp>

#include <vector>
#include <stack>

namespace oly::algo
{
	struct UTFTaggedTextParser
	{
		struct Group
		{
			imp::utf::string str;
			std::stack<imp::utf::string> tags;
		};

		std::vector<Group> groups;

		UTFTaggedTextParser(const imp::utf::string& input);
	};
}
