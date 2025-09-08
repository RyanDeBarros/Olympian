#pragma once

#include "physics/dynamics/bodies/RigidBody.h"
#include "physics/dynamics/components/LinearPhysicsComponent.h"
#include "core/base/ActionDelegate.h"

namespace oly::physics
{
	class LinearBody : public RigidBody
	{
		LinearPhysicsComponent dynamics;

	public:
		ActionDelegator<col2d::CollisionEventData> delegator;

		LinearBody();
		LinearBody(const LinearBody&);
		LinearBody(LinearBody&&) noexcept;
		~LinearBody();

	protected:
		void physics_pre_tick() override;
		void physics_post_tick() override;

	public:
		const MaterialRef& material() const { return dynamics.material; }
		MaterialRef& material() { return dynamics.material; }
		const LinearSubMaterialRef& sub_material() const { return dynamics.submaterial; }
		LinearSubMaterialRef& sub_material() { return dynamics.submaterial; }
		const LinearPhysicsProperties& properties() const { return dynamics.properties; }
		LinearPhysicsProperties& properties() { return dynamics.properties; }

		State state() const override { return dynamics.get_state(); }
		bool is_colliding() const override { return dynamics.is_colliding(); }

	protected:
		void bind(const col2d::Collider& collider) const override;
		void unbind(const col2d::Collider& collider) const override;

		const DynamicsComponent& get_dynamics() const override { return dynamics; }

	public:
		const LinearPhysicsComponent& linear_dynamics() const { return dynamics; }
		LinearPhysicsComponent& linear_dynamics() { return dynamics; }

	private:
		void handle_collides(const col2d::CollisionEventData& data) const;
	};

	typedef SmartReference<LinearBody> LinearBodyRef;
}
