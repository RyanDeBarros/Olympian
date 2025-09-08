#pragma once

#include "physics/dynamics/bodies/RigidBody.h"
#include "physics/dynamics/components/KinematicPhysicsComponent.h"
#include "core/base/ActionDelegate.h"

namespace oly::physics
{
	class KinematicBody : public RigidBody
	{
		KinematicPhysicsComponent dynamics;

	public:
		ActionDelegator<col2d::ContactEventData> delegator;

		KinematicBody();
		KinematicBody(const KinematicBody&);
		KinematicBody(KinematicBody&&) noexcept;
		~KinematicBody();

	protected:
		void physics_pre_tick() override;
		void physics_post_tick() override;

	public:
		const MaterialRef& material() const { return dynamics.material; }
		MaterialRef& material() { return dynamics.material; }
		const KinematicSubMaterialRef& sub_material() const { return dynamics.submaterial; }
		KinematicSubMaterialRef& sub_material() { return dynamics.submaterial; }
		const KinematicPhysicsProperties& properties() const { return dynamics.properties; }
		KinematicPhysicsProperties& properties() { return dynamics.properties; }

		State state() const override { return dynamics.get_state(); }
		bool is_colliding() const override { return dynamics.is_colliding(); }

	protected:
		void bind(const col2d::Collider& collider) const override;
		void unbind(const col2d::Collider& collider) const override;

		const DynamicsComponent& get_dynamics() const override { return dynamics; }

	public:
		const KinematicPhysicsComponent& kinematic_dynamics() const { return dynamics; }
		KinematicPhysicsComponent& kinematic_dynamics() { return dynamics; }

	private:
		void handle_contacts(const col2d::ContactEventData& data) const;
	};

	typedef SmartReference<KinematicBody> KinematicBodyRef;
}
