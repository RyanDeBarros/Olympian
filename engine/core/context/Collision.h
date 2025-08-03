#pragma once

namespace oly
{
	namespace col2d
	{
		class CollisionDispatcher;
	}
}

namespace oly::context
{
	namespace internal
	{
		extern void terminate_collision();
		extern void frame_collision();
	}

	extern col2d::CollisionDispatcher& collision_dispatcher();
}
