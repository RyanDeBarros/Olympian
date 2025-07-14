#pragma once

#include "physics/collision/scene/CollisionDispatcher.h"
#include "physics/collision/objects/Capsule.h"
#include "physics/collision/objects/Polygon.h"
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
		Transformer2DExposure<TExposureParams{ .local = exposure::local::FULL, .chain = exposure::chain::FULL, .modifier = exposure::modifier::FULL }> set_transformer() { return transformer; }
		const Transform2D& get_local() const { return transformer.get_local(); }
		Transform2D& set_local() { return transformer.set_local(); }

		SoftReference<col2d::Collider> add_collider(const col2d::AABB& obj) { return add_collider(col2d::TPrimitive(obj)); }
		SoftReference<col2d::Collider> add_collider(col2d::AABB&& obj) { return add_collider(col2d::TPrimitive(std::move(obj))); }
		SoftReference<col2d::Collider> add_collider(const col2d::OBB& obj) { return add_collider(col2d::TPrimitive(obj)); }
		SoftReference<col2d::Collider> add_collider(col2d::OBB&& obj) { return add_collider(col2d::TPrimitive(std::move(obj))); }
		SoftReference<col2d::Collider> add_collider(const col2d::ConvexHull& obj) { return add_collider(col2d::TPrimitive(obj)); }
		SoftReference<col2d::Collider> add_collider(col2d::ConvexHull&& obj) { return add_collider(col2d::TPrimitive(std::move(obj))); }
		SoftReference<col2d::Collider> add_collider(const col2d::Circle& obj) { return add_collider(col2d::TPrimitive(obj)); }
		SoftReference<col2d::Collider> add_collider(col2d::Circle&& obj) { return add_collider(col2d::TPrimitive(std::move(obj))); }
		template<size_t K>
		SoftReference<col2d::Collider> add_collider(const col2d::KDOP<K>& obj) { return add_collider(col2d::TPrimitive(obj)); }
		template<size_t K>
		SoftReference<col2d::Collider> add_collider(col2d::KDOP<K>&& obj) { return add_collider(col2d::TPrimitive(std::move(obj))); }

		SoftReference<col2d::Collider> add_collider(const col2d::Capsule& capsule) { return add_collider(capsule.tcompound()); }
		SoftReference<col2d::Collider> add_collider(const col2d::PolygonCollision& polygon) { return add_collider(polygon.as_convex_tbvh<col2d::OBB>()); }

		template<typename CObj, typename = std::enable_if_t<col2d::internal::IsColliderObject<std::decay_t<CObj>>>>
		SoftReference<col2d::Collider> add_collider(CObj&& obj) { return add_collider(col2d::Collider(std::forward<CObj>(obj))); }
		SoftReference<col2d::Collider> add_collider(col2d::Collider&& collider);
		
		void erase_collider(size_t i);
		void remove_collider(const SoftReference<col2d::Collider>& collider);
		void clear_colliders();
		SoftReference<col2d::Collider> collider(size_t i = 0);
		size_t num_colliders() const { return colliders.size(); }

		debug::CollisionView collision_view(size_t i, glm::vec4 color) const;
		void update_view(size_t i, debug::CollisionView& view, glm::vec4 color) const;
		void update_view(size_t i, debug::CollisionView& view) const;

		void on_tick();

		const Material& material() const { return dynamics.material; }
		Material& material() { return dynamics.material; }
		const Properties& properties() const { return dynamics.properties; }
		Properties& properties() { return dynamics.properties; }
		State state() const { return dynamics.get_state(); }
		DynamicsComponent::Flag get_flag() const { return dynamics.flag; }
		void set_flag(DynamicsComponent::Flag flag);
		bool is_colliding() const { return dynamics.is_colliding(); }

	private:
		void handle_collides(const col2d::CollisionEventData& data) const;
		void handle_contacts(const col2d::ContactEventData& data) const;

		void bind_collides_handler() const;
		void bind_contacts_handler() const;
		void unbind_collides_handler() const;
		void unbind_contacts_handler() const;
		void bind_by_flag(const col2d::Collider& collider) const;
		void unbind_by_flag(const col2d::Collider& collider) const;
	};
}
