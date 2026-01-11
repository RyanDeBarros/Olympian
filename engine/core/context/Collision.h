#pragma once

#include "external/TOML.h"
#include "physics/collision/elements/Element.h"

namespace oly::context
{
	namespace internal
	{
		extern void init_collision(TOMLNode);
	}
	
    extern col2d::Mask get_collision_mask(const std::string& name);
    extern void set_collision_mask_index(int index, const std::string& name);

    extern col2d::Layer get_collision_layer(const std::string& name);
    extern void set_collision_layer_index(int index, const std::string& name);
}
