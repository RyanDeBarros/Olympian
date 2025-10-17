#pragma once

#include "external/TOML.h"

namespace oly::reg
{
	extern void load_signal(TOMLNode node);
	extern void load_signal_mapping(TOMLNode node);
	extern void load_signals(const char* signal_registry_filepath);
}
