#pragma once

#include "graphics/DrawCommands.h"

namespace oly
{
	namespace rendering
	{
		class DrawCommandRegistry
		{
			std::unordered_map<std::string, DrawCommandList> command_lists;

		public:
			void load(const char* draw_command_registry_file);
			void load(const std::string& draw_command_registry_file) { load(draw_command_registry_file.c_str()); }
			void clear();
			void execute(const std::string& name) const;
		};
	}
}
