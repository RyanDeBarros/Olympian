#pragma once

#include "core/base/ActionDelegate.h"

#include "physics/dynamics/bodies/RigidBody.h"

namespace oly::physics
{
	class StaticBody
        : public RigidBody
	{
    private:
		DynamicsComponent dynamics_;

	public:
		ActionDelegator<col2d::OverlapEventData> delegator;

		StaticBody();
		StaticBody(const StaticBody&) = delete;
		StaticBody(StaticBody&&) noexcept;
		~StaticBody();

        StaticBody& operator=(const StaticBody&) = delete;
        StaticBody& operator=(StaticBody&&) noexcept;

    public:
        State state() const override;
        bool is_colliding() const override;

	protected:
		void physics_pre_tick() override;
		void physics_post_tick() override;

        void bind(const col2d::Collider& collider) const override;
        void unbind(const col2d::Collider& collider) const override;

        const DynamicsComponent& get_dynamics() const override;

	public:
        const DynamicsComponent& dynamics() const;
        DynamicsComponent& dynamics();

		const MaterialRef& material() const;
        MaterialRef& material();

	private:
		void handle_overlaps(const col2d::OverlapEventData& data) const;
	};

	using StaticBodyRef = SmartReference<StaticBody>;
}
