#pragma once

#include "physics/collision/scene/CollisionDispatcher.h"
#include "physics/dynamics/DynamicsComponent.h"

namespace oly::physics
{
	class RigidBody : public col2d::CollisionController
	{
		OLY_COLLISION_CONTROLLER_HEADER(RigidBody);

	private:
		std::vector<CopyPtr<col2d::Collider>> colliders;
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

		// TODO add_collider(CObj&&), add_collider(ElementParam), add_collider(TPrimitive), etc.

		SoftReference<col2d::Collider> add_collider(col2d::Collider&& collider);
		void erase_collider(size_t i);
		void remove_collider(const SoftReference<col2d::Collider>& collider);
		void clear_colliders();
		SoftReference<col2d::Collider> collider(size_t i = 0);
		size_t num_colliders() const { return colliders.size(); }

		debug::CollisionView collision_view(size_t i, glm::vec4 color) const;
		void update_view(size_t i, debug::CollisionView& view, glm::vec4 color) const;

		void on_tick();

		const Material& material() const { return dynamics.material; }
		Material& material() { return dynamics.material; }
		const Properties& properties() const { return dynamics.properties; }
		Properties& properties() { return dynamics.properties; }
		DynamicsComponent::Flag flag() const { return dynamics.flag; }
		DynamicsComponent::Flag& flag() { return dynamics.flag; }
		bool is_colliding() const { return dynamics.is_colliding(); }

	private:
		void handle_contacts(const col2d::ContactEventData& data) const;
	};
}
