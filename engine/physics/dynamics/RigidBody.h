#pragma once

#include "physics/collision/scene/CollisionDispatcher.h"
#include "core/base/TransformerExposure.h"

namespace oly::physics
{
	class SimpleRigidBody : public col2d::CollisionController
	{
		OLY_COLLISION_CONTROLLER_HEADER(SimpleRigidBody);

	private:
		ContiguousSet<ConstSoftReference<col2d::Collider>> colliders;

		Transformer2D transformer;

	public:
		Transformer2DConstExposure get_transformer() const { return transformer; }
		Transformer2DExposure<exposure::FULL> set_transformer() { return transformer; }

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

	class RigidBody : public col2d::CollisionController
	{
		OLY_COLLISION_CONTROLLER_HEADER(RigidBody);

	private:
		ContiguousSet<ConstSoftReference<col2d::Collider>> colliders;

		Transformer2D transformer;

	public:
		Transformer2DConstExposure get_transformer() const { return transformer; }
		Transformer2DExposure<exposure::FULL> set_transformer() { return transformer; }

		RigidBody() = default;
		RigidBody(const RigidBody&);
		RigidBody(RigidBody&&) noexcept;
		~RigidBody();
		RigidBody& operator=(const RigidBody&);
		RigidBody& operator=(RigidBody&&) noexcept;

		void bind_collider(const ConstSoftReference<col2d::Collider>& collider);
		void unbind_collider(const ConstSoftReference<col2d::Collider>& collider);
		void clear_colliders();

	private:
		void handle_contacts(const col2d::ContactEventData& data);
	};
}
