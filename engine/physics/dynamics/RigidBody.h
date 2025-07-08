#pragma once

#include "physics/collision/scene/CollisionDispatcher.h"
#include "physics/dynamics/DynamicsComponent.h"

namespace oly::physics
{
	class RigidBody : public col2d::CollisionController
	{
		OLY_COLLISION_CONTROLLER_HEADER(RigidBody);

	private:
		ContiguousSet<ConstSoftReference<col2d::Collider>> colliders;
		Transformer2D transformer;

		friend class DynamicsComponent;
		DynamicsComponent dynamics;

	public:
		RigidBody() = default;
		RigidBody(const RigidBody&);
		RigidBody(RigidBody&&) noexcept;
		~RigidBody();
		RigidBody& operator=(const RigidBody&);
		RigidBody& operator=(RigidBody&&) noexcept;

		Transformer2DConstExposure get_transformer() const { return transformer; }
		Transformer2DExposure<exposure::FULL> set_transformer() { return transformer; }
		const Transform2D& get_local() const { return transformer.get_local(); }
		Transform2D& set_local() { return transformer.set_local(); }

		void bind_collider(const ConstSoftReference<col2d::Collider>& collider);
		void unbind_collider(const ConstSoftReference<col2d::Collider>& collider);
		void clear_colliders();

		void on_tick();

		const Material& material() const { return dynamics.material; }
		Material& material() { return dynamics.material; }
		const Properties& properties() const { return dynamics.properties; }
		Properties& properties() { return dynamics.properties; }

	private:
		void handle_contacts(const col2d::ContactEventData& data) const;
	};
}
