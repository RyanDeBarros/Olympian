#include "Collision.h"

#include "physics/collision/scene/CollisionDispatcher.h"

namespace oly::context
{
	namespace internal
	{
		col2d::internal::CollisionDispatcher collision_dispatcher;
	}

	void internal::terminate_collision()
	{
		internal::collision_dispatcher.clear();
	}

	void internal::frame_collision()
	{
		internal::collision_dispatcher.poll();
	}

	col2d::internal::CollisionDispatcher& collision_dispatcher()
	{
		return internal::collision_dispatcher;
	}
}
