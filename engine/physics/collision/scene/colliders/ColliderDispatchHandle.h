#pragma once

namespace oly::col2d
{
	class Collider;
}

namespace oly::col2d::internal
{
	struct ColliderDispatchHandle
	{
		const Collider& collider;

		ColliderDispatchHandle(const Collider&);
		ColliderDispatchHandle(const Collider&, const ColliderDispatchHandle&);
		ColliderDispatchHandle(const Collider&, ColliderDispatchHandle&&) noexcept;
		~ColliderDispatchHandle();
		ColliderDispatchHandle& operator=(const ColliderDispatchHandle&);
		ColliderDispatchHandle& operator=(ColliderDispatchHandle&&) noexcept;

	private:
		void copy_handlers(const ColliderDispatchHandle&);
		void move_handlers(ColliderDispatchHandle&&);
		void remove_handlers();
	};
}
