#pragma once

#include "assets/KeyDecl.h"

#include <string>

namespace oly::detail
{
	constexpr size_t KeySize = 8;

	extern std::string encode_key(Key key);
	extern Key decode_key(const std::string& code);
}
