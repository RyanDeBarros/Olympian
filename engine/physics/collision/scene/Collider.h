#pragma once

#include "physics/collision/scene/CollisionTree.h"
#include "core/base/TransformerExposure.h"

namespace oly::physics { class RigidBody; };

namespace oly::col2d
{
	namespace internal
	{
		class TreeHandleMap
		{
			friend class internal::CollisionNode;
			friend class CollisionTree;
			friend class Collider;
			Collider& collider;
			mutable ContiguousMap<const CollisionTree*, internal::CollisionNode*> handles;

			TreeHandleMap(Collider& collider) : collider(collider) {}
			TreeHandleMap(const TreeHandleMap&) = delete;
			TreeHandleMap(TreeHandleMap&&) = delete;
			TreeHandleMap(Collider&, const TreeHandleMap&);
			TreeHandleMap(Collider&, TreeHandleMap&&) noexcept;
			~TreeHandleMap();

			TreeHandleMap& operator=(const TreeHandleMap&);
			TreeHandleMap& operator=(TreeHandleMap&&) noexcept;

			void flush() const;

		public:
			void attach(const CollisionTree& tree);
			void attach(size_t context_tree_index = 0);
			void detach(const CollisionTree& tree);
			void detach(size_t context_tree_index = 0);
			bool is_attached(const CollisionTree& tree) const { return handles.count(&tree); }
			bool is_attached(size_t context_tree_index = 0) const;
			void clear();
			size_t size() const { return handles.size(); }
		};
	}

	// TODO some mechanism to limit the direction that a rigid body/collider can collide with. For example, horizontal-only collision, or vertical-only.

	class Collider
	{
		// LATER movable/static Colliders for optimization in CollisionTree flushing.
		friend class CollisionTree;
		friend class internal::CollisionNode;

		OLY_SOFT_REFERENCE_BASE_DECLARATION(Collider);

	private:
		internal::ColliderObject obj;

		mutable bool dirty = true;

		friend class physics::RigidBody;
		physics::RigidBody* rigid_body = nullptr;

	protected:
		mutable math::Rect2D quad_wrap;

	public:
		internal::TreeHandleMap handles = internal::TreeHandleMap(*this);

		Collider() = default;
		template<typename CObj, typename = std::enable_if_t<internal::IsColliderObject<std::decay_t<CObj>>>>
		explicit Collider(CObj&& obj) : obj(std::forward<CObj>(obj)) {}
		Collider(internal::ColliderObject&& obj) : obj(std::move(obj)) {}
		Collider(const Collider&);
		Collider(Collider&&) noexcept;
		Collider& operator=(const Collider&);
		Collider& operator=(Collider&&) noexcept;

		template<typename CObj, typename = std::enable_if_t<internal::IsColliderObject<std::decay_t<CObj>>>>
		const CObj& get() const { return obj.get<CObj>(); }
		template<typename CObj, typename = std::enable_if_t<internal::IsColliderObject<std::decay_t<CObj>>>>
		CObj& set() { dirty = true; return obj.set<CObj>(); }
		void emplace(internal::ColliderObject&& obj) { dirty = true; this->obj = std::move(obj); }
		template<typename CObj, typename = std::enable_if_t<internal::IsColliderObject<std::decay_t<CObj>>>>
		void emplace(CObj&& obj) { dirty = true; this->obj = internal::ColliderObject(std::forward<CObj>(obj)); }

		void flag() { dirty = true; }

		const Transform2D& get_local() const { return internal::lut_transformer(obj).get_local(); }
		Transform2D& set_local() { return internal::lut_transformer(obj).set_local(); }

		Transformer2DConstExposure get_transformer() const { return internal::lut_transformer(obj); }
		Transformer2DExposure<exposure::FULL> set_transformer() { return internal::lut_transformer(obj); }

	private:
		bool is_dirty() const { return dirty || internal::lut_is_dirty(obj); }
		void flush() const;

	public:
		OverlapResult point_hits(glm::vec2 test) const { return internal::lut_point_hits(obj, test); }
		OverlapResult ray_hits(const Ray& ray) const { return internal::lut_ray_hits(obj, ray); }
		RaycastResult raycast(const Ray& ray) const { return internal::lut_raycast(obj, ray); }
		OverlapResult overlaps(const Collider& other) const { return internal::lut_overlaps(obj, other.obj); }
		CollisionResult collides(const Collider& other) const { return internal::lut_collides(obj, other.obj); }
		ContactResult contacts(const Collider& other) const { return internal::lut_contacts(obj, other.obj); }
		OverlapResult circle_cast_hits(const CircleCast& cast) const { return internal::lut_circle_cast_hits(obj, cast); }
		OverlapResult rect_cast_hits(const RectCast& cast) const { return internal::lut_rect_cast_hits(obj, cast); }

		debug::CollisionView collision_view(glm::vec4 color) const { return internal::lut_collision_view(obj, color); }
		void update_view(debug::CollisionView& view, glm::vec4 color) const { internal::lut_update_view(view, obj, color); }

	private:
		internal::ColliderObjectConstVariant get_object_variant() const { return internal::lut_variant(obj); }
		internal::ColliderObjectVariant get_object_variant() { return internal::lut_variant(obj); }
	};
}
