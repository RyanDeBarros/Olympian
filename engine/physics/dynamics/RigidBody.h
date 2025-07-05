#pragma once

#include "physics/collision/scene/CollisionDispatcher.h"
#include "physics/dynamics/DynamicsComponent.h"

namespace oly::physics
{
	// TODO for complex greedy mtv, maybe keep track of all impulses of maximal length, or even all of them.
	class GreedyMTV
	{
		glm::vec2 mtv = {};
		bool dual = false;
		math::Rect2D contact_box = {};

	public:
		void reset() { mtv = {}; dual = false; contact_box = {}; }
		void add(glm::vec2 impulse, glm::vec2 position)
		{
			float impulse_mag_sqrd = math::mag_sqrd(impulse);
			float mtv_mag_sqrd = math::mag_sqrd(mtv);
			if (col2d::approx(impulse_mag_sqrd, mtv_mag_sqrd))
			{
				if (glm::dot(impulse, mtv) < 0.0f)
					dual = true;

				contact_box.include(position);
			}
			else if (impulse_mag_sqrd > mtv_mag_sqrd)
			{
				mtv = impulse;
				contact_box = { .x1 = position.x, .x2 = position.x, .y1 = position.y, .y2 = position.y };
				dual = false;
			}
		}
		glm::vec2 get_mtv() const { return dual ? glm::vec2{} : mtv; }
		glm::vec2 get_contact() const { return contact_box.center(); }
	};

	class RigidBody : public col2d::CollisionController
	{
		OLY_COLLISION_CONTROLLER_HEADER(RigidBody);

	private:
		ContiguousSet<ConstSoftReference<col2d::Collider>> colliders;
		Transformer2D transformer;

	public:
		DynamicsComponent dynamics;

		RigidBody() = default;
		RigidBody(const RigidBody&);
		RigidBody(RigidBody&&) noexcept;
		~RigidBody();
		RigidBody& operator=(const RigidBody&);
		RigidBody& operator=(RigidBody&&) noexcept;

		Transformer2DConstExposure get_transformer() const { return transformer; }
		Transformer2DExposure<exposure::FULL> set_transformer() { return transformer; }

		void bind_collider(const ConstSoftReference<col2d::Collider>& collider);
		void unbind_collider(const ConstSoftReference<col2d::Collider>& collider);
		void clear_colliders();

		void on_tick();

	private:
		void handle_contacts(const col2d::ContactEventData& data);
	};
}
