#pragma once

#pragma warning(push)
#pragma warning(disable : 6011)
#pragma warning(disable : 26450)
#pragma warning(disable : 26495)
#pragma warning(disable : 26800)
#pragma warning(disable : 26813)
#pragma warning(disable : 26827)
#include <toml/toml.hpp>
#pragma warning(pop)

namespace oly
{
	typedef toml::node_view<toml::node> TOMLNode;
	typedef toml::node_view<const toml::node> CTOMLNode; // TODO v5 remove
}
