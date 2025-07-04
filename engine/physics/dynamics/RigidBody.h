#pragma once

#include "physics/collision/scene/CollisionDispatcher.h"

namespace oly::physics
{
	class SimpleRigidBody : public col2d::CollisionController
	{
		OLY_COLLISION_CONTROLLER_HEADER(SimpleRigidBody);

	private:
		ContiguousSet<ConstSoftReference<col2d::Collider>> colliders;

		Transformer2D _transformer;

	public:
		// TODO expose transformer methods instead. Since this is a common demand, create helper adapter class(es) that hides a reference to transformer, but exposes certain functions.
		const Transformer2D& transformer() const { return _transformer; }
		Transformer2D& transformer() { return _transformer; }

		SimpleRigidBody() = default;
		SimpleRigidBody(const SimpleRigidBody&);
		SimpleRigidBody(SimpleRigidBody&&) noexcept;
		~SimpleRigidBody();
		SimpleRigidBody& operator=(const SimpleRigidBody&);
		SimpleRigidBody& operator=(SimpleRigidBody&&) noexcept;

		void bind_collider(const ConstSoftReference<col2d::Collider>& collider);
		void unbind_collider(const ConstSoftReference<col2d::Collider>& collider);
		void clear_colliders();

	private:
		void handle_collides(const col2d::CollisionEventData& data);
	};

	// TODO RigidBody uses handle_contacts for torque movement.
}
