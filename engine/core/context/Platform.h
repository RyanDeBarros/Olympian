#pragma once

#include "core/platform/Platform.h"
#include "core/platform/WindowResize.h"
#include "core/platform/BindingContext.h"

namespace oly::context
{
	namespace internal
	{
		extern void init_platform(const TOMLNode&);
		extern void init_viewport(const TOMLNode&);
		extern void terminate_platform();
		extern bool frame_platform();
	}

	extern platform::Platform& get_platform();
	extern platform::WRDrawer& get_wr_drawer();
	
	extern glm::vec2 get_cursor_screen_pos();
	extern glm::vec2 get_initial_window_size();
	extern glm::vec2 get_view_stretch();
	extern glm::vec2 get_cursor_view_pos();

	extern input::internal::InputBindingContext& input_binding_context();
	extern input::SignalTable& signal_table();
	extern input::SignalMappingTable& signal_mapping_table();

	extern void assign_signal_mapping(const std::string& mapping_name, std::vector<std::string>&& signal_names);
	extern void unassign_signal_mapping(const std::string& mapping_name);
}
