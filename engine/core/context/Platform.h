#pragma once

#include "core/platform/Platform.h"
#include "core/platform/BindingContext.h"
#include "core/util/ResourcePath.h"

namespace oly::context
{
	namespace internal
	{
		extern void init_platform(TOMLNode);
		extern void init_viewport(TOMLNode);
		extern void terminate_platform();
		extern bool frame_platform();
	}

	extern platform::Platform& get_platform();

	extern input::internal::InputBindingContext& input_binding_context();
	extern input::SignalTable& signal_table();
	extern input::SignalMappingTable& signal_mapping_table();

	extern void assign_signal_mapping(const std::string& mapping_name, std::vector<std::string>&& signal_names);
	extern void unassign_signal_mapping(const std::string& mapping_name);

	extern void load_signal(TOMLNode node);
	extern void load_signal_mapping(TOMLNode node);
	extern void load_signals(const ResourcePath& file);
}
