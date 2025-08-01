#pragma once

#include "external/TOML.h"

namespace oly::reg
{
	extern void load_signal(const CTOMLNode& node);
	extern void load_signal_mapping(const CTOMLNode& node);
	extern void load_signals(const char* signal_registry_filepath);
}
