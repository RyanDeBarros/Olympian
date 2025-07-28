#pragma once

#include "physics/collision/scene/CollisionDispatcher.h"
#include "physics/collision/objects/Capsule.h"
#include "physics/collision/objects/Polygon.h"
#include "physics/dynamics/DynamicsComponent.h"

#include "core/types/SmartHandle.h"

namespace oly::physics
{
	namespace internal { class RigidBodyManager; }

	class RigidBody : public col2d::CollisionController
	{
		OLY_COLLISION_CONTROLLER_HEADER(RigidBody);

	protected:
		std::vector<CopyPtr<col2d::Collider>> colliders;
		Transformer2D transformer;

	public:
		RigidBody();
		RigidBody(const RigidBody&);
		RigidBody(RigidBody&&) noexcept;
		virtual ~RigidBody();
		RigidBody& operator=(const RigidBody&);
		RigidBody& operator=(RigidBody&&) noexcept;

		Transformer2DConstExposure get_transformer() const { return transformer; }
		Transformer2DExposure<TExposureParams{ .local = exposure::local::FULL, .chain = exposure::chain::FULL, .modifier = exposure::modifier::FULL }> set_transformer() { return transformer; }
		const Transform2D& get_local() const { return transformer.get_local(); }
		Transform2D& set_local() { return transformer.set_local(); }

		template<col2d::internal::ElementShape Shape>
		SoftReference<col2d::Collider> add_collider(Shape&& obj) { return add_collider(col2d::TPrimitive(std::forward<Shape>(obj))); }
		SoftReference<col2d::Collider> add_collider(const col2d::Capsule& capsule) { return add_collider(capsule.tcompound()); }
		SoftReference<col2d::Collider> add_collider(const col2d::PolygonCollision& polygon) { return add_collider(polygon.as_convex_tbvh<col2d::OBB>()); }

		template<col2d::internal::ColliderObjectShape CObj>
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

	private:
		friend class internal::RigidBodyManager;
		virtual void physics_pre_tick() = 0;
		virtual void physics_post_tick() = 0;

	public:
		virtual State state() const = 0;
		virtual bool is_colliding() const = 0;

	protected:
		virtual void bind(const col2d::Collider& collider) const = 0;
		virtual void unbind(const col2d::Collider& collider) const = 0;
		void bind_all() const;
		void unbind_all() const;

		virtual const DynamicsComponent& get_dynamics() const = 0;
		const RigidBody* rigid_body(const col2d::Collider& collider) const;
		const DynamicsComponent& dynamics_of(const RigidBody& other) const { return other.get_dynamics(); }
	};

	namespace internal
	{
		class RigidBodyManager
		{
			friend class RigidBody;
			std::unordered_set<RigidBody*> rigid_bodies;

			RigidBodyManager() = default;
			RigidBodyManager(const RigidBodyManager&) = delete;
			RigidBodyManager(RigidBodyManager&&) = delete;
			~RigidBodyManager() { clear(); }

		public:
			static RigidBodyManager& instance()
			{
				static RigidBodyManager manager;
				return manager;
			}

			void on_tick() const;

			void clear()
			{
				rigid_bodies.clear();
			}
		};
	}

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

	typedef SmartHandle<StaticBody> StaticBodyRef;

	class LinearBody : public RigidBody
	{
		OLY_COLLISION_CONTROLLER_HEADER(LinearBody);

	private:
		LinearPhysicsComponent dynamics;

	public:
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
		const LinearPhysicsProperties& properties() const { return dynamics.properties; }
		LinearPhysicsProperties& properties() { return dynamics.properties; }

		State state() const override { return dynamics.get_state(); }
		bool is_colliding() const override { return dynamics.is_colliding(); }

	protected:
		void bind(const col2d::Collider& collider) const override;
		void unbind(const col2d::Collider& collider) const override;

		const DynamicsComponent& get_dynamics() const override { return dynamics; }

	private:
		void handle_collides(const col2d::CollisionEventData& data) const;
	};

	typedef SmartHandle<LinearBody> LinearBodyRef;

	class KinematicBody : public RigidBody
	{
		OLY_COLLISION_CONTROLLER_HEADER(KinematicBody);

	private:
		KinematicPhysicsComponent dynamics;

	public:
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
		const KinematicPhysicsProperties& properties() const { return dynamics.properties; }
		KinematicPhysicsProperties& properties() { return dynamics.properties; }

		State state() const override { return dynamics.get_state(); }
		bool is_colliding() const override { return dynamics.is_colliding(); }

	protected:
		void bind(const col2d::Collider& collider) const override;
		void unbind(const col2d::Collider& collider) const override;

		const DynamicsComponent& get_dynamics() const override { return dynamics; }

	private:
		void handle_contacts(const col2d::ContactEventData& data) const;
	};

	typedef SmartHandle<KinematicBody> KinematicBodyRef;
}
