#pragma once

#include "core/base/TransformerExposure.h"
#include "core/base/Parameters.h"
#include "physics/collision/scene/Collider.h"
#include "physics/dynamics/Material.h"

namespace oly::physics
{
	struct State
	{
		glm::vec2 position = {};
		float rotation = 0.0f;
		glm::vec2 linear_velocity = {};
		float angular_velocity = 0.0f;
	};

	extern FrictionType friction_type(UnitVector2D tangent, glm::vec2 relative_contact_velocity,
		glm::vec2 relative_linear_velocity, float angular_velocity_A, float angular_velocity_B,
		PositiveFloat speed_threshold = (float)col2d::LINEAR_TOLERANCE);

	class DynamicsComponent;

	struct CollisionResponse
	{
		glm::vec2 mtv;
		glm::vec2 contact;
		UnitVector2D normal;
		const DynamicsComponent* dynamics;
	};

	class DynamicsComponent
	{
	protected:
		mutable std::vector<CollisionResponse> collisions;
		mutable bool was_colliding = false;

		mutable State pre_state;
		mutable State post_state;

	public:
		// TODO v3 only restitution and friction are needed. Make CollisionDamping, Blending, AngularSnapping, and LinearSnapping optional components in Material.
		// The optional-ness can be set in constructor - Create FixedOptional struct that is an std::optional but once set/not set, cannot be set/unset. Then in StaticPhysics/LinearPhysics/KinematicPhysics, construct it with proper state.
		MaterialRef material = REF_INIT;

		virtual ~DynamicsComponent() = default;

	protected:
		virtual std::optional<float> teleport_mass() const { return std::nullopt; }
		virtual float eff_mass_denom_factor(glm::vec2 local_contact, UnitVector2D normal) const { return 0.0f; }
		float effective_mass(const CollisionResponse& collision) const;

		virtual glm::vec2 contact_velocity(glm::vec2 local_contact) const { return pre_state.linear_velocity; }
		glm::vec2 other_contact_velocity(const CollisionResponse& collision) const;
		glm::vec2 relative_contact_velocity(const CollisionResponse& collision) const;
		float restitution_with(const CollisionResponse& collision) const;
		float friction_with(const CollisionResponse& collision) const;

	public:
		State get_state() const { return post_state; }

		void add_collision(glm::vec2 mtv, glm::vec2 contact, const DynamicsComponent& other) const;
		bool is_colliding() const { return was_colliding; }

		void pre_tick(const glm::mat3& global) const;
		virtual void post_tick() const;

	protected:
		float teleport_factor(const DynamicsComponent& other) const;
	};

	class LinearPhysicsComponent;

	struct LinearPhysicsProperties
	{
	private:
		StrictlyPositiveFloat _mass = 1.0f;
		StrictlyPositiveFloat _mass_inverse = 1.0f;

	public:
		float mass() const { return _mass; }
		float mass_inverse() const { return _mass_inverse; }
		void set_mass(float m) { _mass = m; _mass_inverse = 1.0f / _mass; }

	private:
		std::vector<glm::vec2> applied_accelerations;
		std::vector<glm::vec2> applied_forces;

	public:
		mutable std::vector<glm::vec2> applied_impulses;

		glm::vec2 net_acceleration = {}; // does not include applied accelerations
		glm::vec2 net_force = {}; // does not include applied forces
		mutable glm::vec2 net_impulse = {}; // does not include applied impulses

		bool complex_teleportation = false;

		struct
		{
			bool enable = false;
			bool only_colliding = false;
		} linear_x_snapping;

		struct
		{
			bool enable = false;
			bool only_colliding = false;
		} linear_y_snapping;

	private:
		friend class LinearPhysicsComponent;
		glm::vec2 dv_psi() const;

		mutable glm::vec2 _net_applied_acceleration = {};
		mutable glm::vec2 _net_applied_force = {};
		mutable bool dirty_applied_accelerations = false;
		mutable bool dirty_applied_forces = false;

	public:
		const std::vector<glm::vec2>& get_applied_accelerations() const { return applied_accelerations; }
		std::vector<glm::vec2>& set_applied_accelerations() { dirty_applied_accelerations = true; return applied_accelerations; }
		const std::vector<glm::vec2>& get_applied_forces() const { return applied_forces; }
		std::vector<glm::vec2>& set_applied_forces() { dirty_applied_forces = true; return applied_forces; }
	};

	class LinearPhysicsComponent : public DynamicsComponent
	{
		mutable size_t primary_collision_mtv_idx = 0;
		mutable size_t secondary_collision_mtv_idx = 0;
		mutable bool found_secondary_collision_mtv_idx = false;
		mutable glm::vec2 collision_linear_impulse = {};

	public:
		LinearPhysicsProperties properties;

		void post_tick() const override;

	protected:
		std::optional<float> teleport_mass() const override { return properties.mass(); }
		float eff_mass_denom_factor(glm::vec2 local_contact, UnitVector2D normal) const override;

	private:
		void update_colliding_linear_motion(glm::vec2 new_velocity) const;
		void compute_collision_mtv_idxs() const;

		void compute_collision_response(glm::vec2 new_velocity) const;
		glm::vec2 restitution_impulse(const CollisionResponse& collision, float eff_mass) const;
		glm::vec2 friction_impulse(const CollisionResponse& collision, float eff_mass, glm::vec2 new_linear_velocity) const;

		void snap_motion() const;
	};

	struct AppliedAcceleration
	{
		glm::vec2 acceleration = {};
		glm::vec2 contact = {};
	};

	struct AppliedForce
	{
		glm::vec2 force = {};
		glm::vec2 contact = {};
	};

	struct AppliedImpulse
	{
		glm::vec2 impulse = {};
		glm::vec2 contact = {};
	};

	class KinematicPhysicsComponent;

	struct KinematicPhysicsProperties
	{
	private:
		StrictlyPositiveFloat _mass = 1.0f;
		StrictlyPositiveFloat _mass_inverse = 1.0f;

		StrictlyPositiveFloat _moi = 1.0f;
		StrictlyPositiveFloat _moi_inverse = 1.0f;

		StrictlyPositiveFloat _moi_multiplier = 1.0f;

	public:
		float mass() const { return _mass; }
		float mass_inverse() const { return _mass_inverse; }
		void set_mass(float m);

		float moi() const { return _moi; }
		float moi_inverse() const { return _moi_inverse; }
		void set_moi(float m);

		bool use_moi_multiplier = true;
		float moi_multiplier() const { return use_moi_multiplier ? (float)_moi_multiplier : _moi * _mass_inverse; }
		void set_moi_multiplier(float mult);

	private:
		std::vector<AppliedAcceleration> applied_accelerations;
		std::vector<AppliedForce> applied_forces;

	public:
		mutable std::vector<AppliedImpulse> applied_impulses;
		glm::vec2 net_linear_acceleration = {}; // does not include applied accelerations
		glm::vec2 net_force = {}; // does not include applied forces
		mutable glm::vec2 net_linear_impulse = {}; // does not include applied impulses
		float net_angular_acceleration = 0.0f;
		float net_torque = 0.0f;
		mutable float net_angular_impulse = 0.0f;

		glm::vec2 center_of_mass = {};

		bool complex_teleportation = false;

		struct
		{
			bool enable = false;
			bool only_colliding = false;
		} angular_snapping;

		struct
		{
			bool enable = false;
			bool only_colliding = false;
		} linear_x_snapping;

		struct
		{
			bool enable = false;
			bool only_colliding = false;
		} linear_y_snapping;

	private:
		friend class KinematicPhysicsComponent;
		glm::vec2 dv_psi() const;
		float dw_psi() const;

		mutable glm::vec2 _net_linear_applied_acceleration = {};
		mutable float _net_angular_applied_acceleration = 0.0f;
		mutable glm::vec2 _net_linear_applied_force = {};
		mutable float _net_angular_applied_force = 0.0f;
		mutable bool dirty_linear_applied_accelerations = false;
		mutable bool dirty_angular_applied_accelerations = false;
		mutable bool dirty_linear_applied_forces = false;
		mutable bool dirty_angular_applied_forces = false;

	public:
		const std::vector<AppliedAcceleration>& get_applied_accelerations() const { return applied_accelerations; }
		std::vector<AppliedAcceleration>& set_applied_accelerations() { dirty_linear_applied_accelerations = true; dirty_angular_applied_accelerations = true; return applied_accelerations; }
		const std::vector<AppliedForce>& get_applied_forces() const { return applied_forces; }
		std::vector<AppliedForce>& set_applied_forces() { dirty_linear_applied_forces = true; dirty_angular_applied_forces = true; return applied_forces; }
	};

	class KinematicPhysicsComponent : public DynamicsComponent
	{
		mutable size_t primary_collision_mtv_idx = 0;
		mutable size_t secondary_collision_mtv_idx = 0;
		mutable bool found_secondary_collision_mtv_idx = false;
		mutable glm::vec2 collision_linear_impulse = {};
		mutable float collision_angular_impulse = 0.0f;

	public:
		KinematicPhysicsProperties properties;

		void post_tick() const override;

	protected:
		std::optional<float> teleport_mass() const override { return properties.mass(); }
		float eff_mass_denom_factor(glm::vec2 local_contact, UnitVector2D normal) const override;
		virtual glm::vec2 contact_velocity(glm::vec2 local_contact) const override;
		
	private:
		void update_colliding_linear_motion(glm::vec2 new_velocity) const;
		void update_colliding_angular_motion(float new_velocity) const;
		void compute_collision_mtv_idxs() const;

		void compute_collision_response(glm::vec2 new_linear_velocity, float new_angular_velocity) const;
		glm::vec2 restitution_impulse(const CollisionResponse& collision, float eff_mass) const;
		glm::vec2 friction_impulse(const CollisionResponse& collision, float eff_mass, glm::vec2 new_linear_velocity, float new_angular_velocity) const;

		void snap_motion() const;
	};
}
