#pragma once

#include "core/base/TransformerExposure.h"
#include "physics/collision/scene/colliders/TreeHandleMap.h"
#include "physics/collision/scene/colliders/ColliderDispatchHandle.h"
#include "physics/collision/scene/colliders/ColliderObject.h"
#include "physics/collision/scene/luts/LUT.h"
#include "physics/collision/scene/luts/LUTVariant.h"

namespace oly::physics { class RigidBody; };

namespace oly::col2d
{
	class Collider
	{
		friend class CollisionTree;
		friend class internal::CollisionNode;

	private:
		internal::ColliderObject obj;

		mutable bool dirty = true;

		friend class physics::RigidBody;
		physics::RigidBody* rigid_body = nullptr;

		internal::ColliderDispatchHandle dispatch_handle;

	protected:
		mutable math::Rect2D quad_wrap;

	public:
		internal::TreeHandleMap handles = internal::TreeHandleMap(*this);

		Collider() : dispatch_handle(*this) {}
		template<internal::ColliderObjectShape CObj>
		explicit Collider(CObj&& obj) : obj(std::forward<CObj>(obj)), dispatch_handle(*this) {}
		Collider(internal::ColliderObject&& obj) : obj(std::move(obj)), dispatch_handle(*this) {}
		Collider(const Collider&);
		Collider(Collider&&) noexcept;
		Collider& operator=(const Collider&);
		Collider& operator=(Collider&&) noexcept;

		template<internal::ColliderObjectShape CObj>
		const CObj& get() const { return obj.get<CObj>(); }
		template<internal::ColliderObjectShape CObj>
		CObj& set() { dirty = true; return obj.set<CObj>(); }
		void emplace(internal::ColliderObject&& obj) { dirty = true; this->obj = std::move(obj); }
		template<internal::ColliderObjectShape CObj>
		void emplace(CObj&& obj) { dirty = true; this->obj = internal::ColliderObject(std::forward<CObj>(obj)); }

		void flag() { dirty = true; }

		const Transform2D& get_local() const { return internal::lut_transformer(obj).get_local(); }
		Transform2D& set_local() { return internal::lut_transformer(obj).set_local(); }

		Transformer2DConstExposure get_transformer() const { return internal::lut_transformer(obj); }
		Transformer2DExposure<TExposureParams{ .local = exposure::local::FULL, .chain = exposure::chain::FULL, .modifier = exposure::modifier::FULL }>
			set_transformer() { return internal::lut_transformer(obj); }

		Layer layer() const { return internal::lut_layer(obj); }
		Layer& layer() { return internal::lut_layer(obj); }
		Mask mask() const { return internal::lut_mask(obj); }
		Mask& mask() { return internal::lut_mask(obj); }

		std::optional<UnitVector2D> one_way_blocking;
		bool one_way_blocks(const Collider& active) const;

	private:
		bool is_dirty() const { return dirty || internal::lut_is_dirty(obj); }
		void flush() const;

	public:
		OverlapResult point_hits(glm::vec2 test) const { return internal::lut_point_hits(obj, test); }
		OverlapResult ray_hits(Ray ray) const { return internal::lut_ray_hits(obj, ray); }
		RaycastResult raycast(Ray ray) const { return internal::lut_raycast(obj, ray); }
		OverlapResult overlaps(const Collider& other) const { return internal::lut_overlaps(obj, other.obj); }
		CollisionResult collides(const Collider& other) const { return internal::lut_collides(obj, other.obj); }
		ContactResult contacts(const Collider& other) const { return internal::lut_contacts(obj, other.obj); }
		OverlapResult circle_cast_hits(const CircleCast& cast) const { return internal::lut_circle_cast_hits(obj, cast); }
		OverlapResult rect_cast_hits(const RectCast& cast) const { return internal::lut_rect_cast_hits(obj, cast); }

		debug::CollisionView collision_view(glm::vec4 color) const { return internal::lut_collision_view(obj, color); }
		void update_view(debug::CollisionView& view, glm::vec4 color, size_t view_index = 0) const { internal::lut_update_view(view, obj, color, view_index); }
		void update_view(debug::CollisionView& view, size_t view_index = 0) const { internal::lut_update_view_no_color(view, obj, view_index); }

	private:
		internal::ColliderObjectConstVariant get_object_variant() const { return internal::lut_variant(obj); }
		internal::ColliderObjectVariant get_object_variant() { return internal::lut_variant(obj); }
	};
}
