#pragma once

#include "external/TOML.h"
#include "physics/collision/elements/Element.h"

namespace oly::col2d::internal
{
	class CollisionDispatcher;
}

namespace oly::context
{
	namespace internal
	{
		extern void init_collision(const TOMLNode&);
		extern void terminate_collision();
		extern void frame_collision();
	}

	extern col2d::internal::CollisionDispatcher& collision_dispatcher();

    extern col2d::Mask get_collision_mask(const std::string& name);
    extern void set_collision_mask_index(int index, const std::string& name);

    extern col2d::Layer get_collision_layer(const std::string& name);
    extern void set_collision_layer_index(int index, const std::string& name);
}
