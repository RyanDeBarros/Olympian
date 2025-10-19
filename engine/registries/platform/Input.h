#pragma once

#include "external/TOML.h"
#include "core/util/ResourcePath.h"

namespace oly::reg
{
	extern void load_signal(TOMLNode node);
	extern void load_signal_mapping(TOMLNode node);
	extern void load_signals(const ResourcePath& file);
}
