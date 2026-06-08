#pragma once

#include "core/platform/Platform.h"
#include "core/platform/BindingContext.h"
#include "core/util/DebugTrace.h"

#include "assets/ResourcePath.h"

namespace oly::context
{
	namespace internal
	{
		extern void init_platform(TOMLNode);
		extern void init_viewport(TOMLNode);
		extern bool platform_frame();
	}

	extern platform::Platform& get_platform();

	extern input::internal::InputBindingContext& input_binding_context();
}
