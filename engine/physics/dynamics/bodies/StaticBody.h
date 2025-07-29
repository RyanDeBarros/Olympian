#pragma once

#include "physics/dynamics/bodies/RigidBody.h"

namespace oly::physics
{
	class StaticBody : public RigidBody
	{
		OLY_COLLISION_CONTROLLER_HEADER(StaticBody);

	private:
		DynamicsComponent dynamics;

	public:
		StaticBody();
		StaticBody(const StaticBody&);
		StaticBody(StaticBody&&) noexcept;
		~StaticBody();

	protected:
		void physics_pre_tick() override;
		void physics_post_tick() override;

	public:
		const MaterialRef& material() const { return dynamics.material; }
		MaterialRef& material() { return dynamics.material; }

		State state() const override { return dynamics.get_state(); }
		bool is_colliding() const override { return dynamics.is_colliding(); }

	protected:
		void bind(const col2d::Collider& collider) const override;
		void unbind(const col2d::Collider& collider) const override;

		const DynamicsComponent& get_dynamics() const override { return dynamics; }

	private:
		void handle_overlaps(const col2d::OverlapEventData& data) const;
	};

	typedef SmartReference<StaticBody> StaticBodyRef;
}
