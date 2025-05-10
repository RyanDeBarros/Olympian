#pragma once

#include "external/TOML.h"

#include "core/platform/Input.h"

namespace oly::reg
{
	extern void load_signal(const TOMLNode& node);
	extern void load_signals(const char* signal_registry_filepath);
}
