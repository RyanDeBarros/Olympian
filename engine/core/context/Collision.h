#pragma once

namespace oly::col2d::internal
{
	class CollisionDispatcher;
}

namespace oly::context
{
	namespace internal
	{
		extern void terminate_collision();
		extern void frame_collision();
	}

	extern col2d::internal::CollisionDispatcher& collision_dispatcher();
}
